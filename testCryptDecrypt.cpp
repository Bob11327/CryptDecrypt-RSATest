//===========================================================================
// testCryptDecrypt.cpp
//===========================================================================
// This sample application demonstrates encrypting data in a specific way:
//		- using CryptoAPI (not newer CNG/NCrypt API)
//		- using "Microsoft Enhanced RSA and AES Cryptographic Provider" container
//		- using the PROV_RSA_AES provider
//		- using the AT_KEYEXCHANGE key for encryption & decryption
//---------------------------------------------------------------------------
// In this test scenario, a blob of data is encrypted with padding to key size (128 bytes).
// When the data is decrypted, the CRYPT_DECRYPT_RSA_NO_PADDING_CHECK flag
// is used to get the entire data with padding, rather than just the
// original data back.
//
// This code produces the expected results on Windows "desktop" OSes 8.1 and 10,
// and on Windows Server OSes 2012R2 and 2016.
//
// INCORRECT RESULTS are returned on Windows 8 and earlier desktop OSes, and
// on Windows Server 2012 and earlier.
//===========================================================================

#include "stdafx.h"
#include <windows.h>
#include <assert.h>
#include <vector>
#include <iostream>
#include <iomanip>

using namespace std;

struct CryptStuff
{
    HCRYPTPROV* phProv;
    HCRYPTKEY* phKey;

    CryptStuff(HCRYPTPROV* hprov, HCRYPTKEY* hkey) :
        phProv(hprov), phKey(hkey) {}

    ~CryptStuff()
    {
        if (*phKey) CryptDestroyKey(*phKey);
        if (*phProv) CryptReleaseContext(*phProv, 0);
    }
};

void EncryptData(vector<BYTE>& data)
{
    HCRYPTPROV hProv = 0;
    HCRYPTKEY hKey = 0;

    LPCTSTR pszContainerName = _T("Test CryptDecrypt Key Container 2");

    CryptStuff cs(&hProv, &hKey);

    if (!CryptAcquireContext(&hProv, pszContainerName, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, 0))
    {
        DWORD err = ::GetLastError();
        if (err == NTE_BAD_KEYSET)
        {
            if (!CryptAcquireContext(&hProv, pszContainerName, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_NEWKEYSET))
            {
                err = ::GetLastError();
                cout << "CryptAcquireContext Error: " << err << endl;
                return;
            }
        }
        else
        {
            cout << "CryptAcquireContext Error: " << err << endl;
            return;
        }
    }

    if (!CryptGetUserKey(hProv, AT_KEYEXCHANGE, &hKey))
    {
        DWORD err = ::GetLastError();
        if (err == NTE_NO_KEY)
        {
            if (!CryptGenKey(hProv, AT_KEYEXCHANGE, CRYPT_EXPORTABLE | CRYPT_NO_SALT, &hKey))
            {
                err = ::GetLastError();
                cout << "CryptGetUserKey Error: " << err << endl;
                return;
            }
        }
        else
        {
            cout << "CryptGetUserKey Error: " << err << endl;
            return;
        }
    }

    DWORD dwValLen = sizeof(DWORD);
    DWORD dwKeyLen = 0;
    if (!CryptGetKeyParam(hKey, KP_KEYLEN, (LPBYTE)&dwKeyLen, &dwValLen, 0))
    {
        DWORD err = ::GetLastError();
        cout << "CryptGetKeyParam(KP_KEYLEN) Error: " << err << endl;
        return;
    }

    DWORD block_size = (dwKeyLen + 7) / 8;
    DWORD data_size = (DWORD)data.size();
    DWORD buffer_size = data_size;

    if (!CryptEncrypt(hKey, 0, TRUE, 0, NULL, &buffer_size, 0))
    {
        DWORD err = ::GetLastError();
        cout << "CryptEncrypt Error: " << err << endl;
        return;
    }

    data.resize(buffer_size);

    if (!CryptEncrypt(hKey, 0, TRUE, 0, &data[0], &data_size, (DWORD)data.size()))
    {
        DWORD err = ::GetLastError();
        cout << "CryptEncrypt Error: " << err << endl;
        return;
    }

    if (!CryptDecrypt(hKey, 0, TRUE, CRYPT_DECRYPT_RSA_NO_PADDING_CHECK, &data[0], &data_size))
    {
        DWORD err = ::GetLastError();
        cout << "CryptDecrypt Error: " << err << endl;
        return;
    }
}

int main()
{
	// Input vector is 48 bytes.  We believe the contents here aren't important, although this particular
	// sequence was captured from an SSL connection negotiation to be representative of the inputs we're expecting
	//
    vector<BYTE> input{ 0x03, 0x03, 0x9A, 0x92, 0x1E, 0x1D, 0xBD, 0x74, 0x68, 0xB1, 0x26, 0x1D, 0xE5, 0xD5, 0xBA, 0xB8, 0xBC, 0x1F, 0xA4, 0x60, 0x00, 0xE4, 0x17, 0xA1, 0x37, 0x38, 0x62, 0xC2, 0xF3, 0xDE, 0x44, 0x2D, 0x45, 0x81, 0x03, 0x1D, 0x8A, 0xBF, 0x69, 0x7F, 0x55, 0x76, 0x0B, 0x5F, 0xB8, 0x33, 0x09, 0x4C };
	assert(input.size() == 48);
	vector<BYTE> originaldata(input);

    EncryptData(input);

    for (BYTE b : input)
    {
        cout << "0x" << setfill('0') << setw(2) << hex << int(b) << ", ";
    }
	cout << "\n";

	// Note well: expected output is in the form:
	//
	// 0x00, 0x02, 0xa9, 0x90, 0x8a, 0x0c, 0xbd, 0xe6, 0x9a, 0x6b, 0x19, 0xe0, 0x91, 0x78, 0x8d, 0x66, 0x7a, 0x0a, 0xcb, 0xfc, 0xbc, 0xd4, 0xce, 0xcc, 0x9c, 0xa7, 0x0f, 0xf7, 0x7f, 0x33, 0x90, 0xc3, 0xff, 0x7b, 0x12, 0x9d, 0xc1, 0x55, 0xee, 0xe8, 0x58, 0x39, 0xbb, 0x1a, 0x5c, 0x82, 0xa7, 0x69, 0xeb, 0xa7, 0x96, 0xf7, 0xad, 0x18, 0x67, 0x2f, 0xd4, 0x8c, 0x4a, 0xb3, 0x0f, 0x75, 0x4a, 0x80, 0x7a, 0x80, 0x87, 0xca, 0x78, 0x0b, 0xd2, 0x08, 0x79, 0x22, 0x2f, 0xab, 0xc1, 0x4e, 0xc5, 0x00, 0x03, 0x03, 0x9a, 0x92, 0x1e, 0x1d, 0xbd, 0x74, 0x68, 0xb1, 0x26, 0x1d, 0xe5, 0xd5, 0xba, 0xb8, 0xbc, 0x1f, 0xa4, 0x60, 0x00, 0xe4, 0x17, 0xa1, 0x37, 0x38, 0x62, 0xc2, 0xf3, 0xde, 0x44, 0x2d, 0x45, 0x81, 0x03, 0x1d, 0x8a, 0xbf, 0x69, 0x7f, 0x55, 0x76, 0x0b, 0x5f, 0xb8, 0x33, 0x09, 0x4c,
	//
	// Which is:
	// The sentinal value 0x00, 0x02 -- indicating that this is an integer value
	// Random filler data of 78 bytes, last byte of which is zero, making total output buffer size 128 bytes
	// The original 48 bytes of input data

	BOOL bGotExpectedResult = TRUE;
#define UpdateExpectedResult(expr)	bGotExpectedResult = bGotExpectedResult && (expr); assert(expr);

	UpdateExpectedResult(input.size() == 128);
	UpdateExpectedResult(input[0] == 0x00);
	UpdateExpectedResult(input[1] == 0x02);
	UpdateExpectedResult(input[79] == 0x00);
	for (size_t ii = 0; ii < originaldata.size(); ++ii)
	{
		UpdateExpectedResult(input[ii + 80] == originaldata[ii]);
	}

	if (bGotExpectedResult)
	{
		cout << "\n\nThis is the expected result.\n";
	}
	else
	{
		cout << "\n\n** NOTE WELL: THIS IS NOT THE EXPECTED RESULT **\n";
	}

    return 0;
}

