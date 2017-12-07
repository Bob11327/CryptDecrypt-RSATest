# CryptDecrypt-RSATest
Console application demonstrates unexpected results with CryptDecrypt API with RSA encryption

This sample application demonstrates encrypting data in a specific way:

* using CryptoAPI (not newer CNG/NCrypt API)
* using "Microsoft Enhanced RSA and AES Cryptographic Provider" container
* using the PROV\_RSA\_AES provider
* using the AT\_KEYEXCHANGE key for encryption & decryption

In this test scenario, a blob of data is encrypted with padding to key size (128 bytes).
When the data is decrypted, the CRYPT\_DECRYPT\_RSA\_NO\_PADDING_CHECK flag
is used to get the entire data with padding, rather than just the
original data back.

This code produces the expected results on Windows "desktop" OSes 8.1 and 10,
and on Windows Server OSes 2012R2 and 2016.

INCORRECT RESULTS are returned on Windows 8 and earlier desktop OSes, and
on Windows Server 2012 and earlier.
