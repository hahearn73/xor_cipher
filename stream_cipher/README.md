# XOR Cipher
This is a simple xor stream cipher.

### Compilation
Run `make` to create executable and `make clean` to clean directory,

### Execution
```
$ ./stream_cipher <-e|-d> <key> <source_file> <destination_file>
```
where `-e` specifies encryption and `-d` specifies decryption. Key must be of length 1 (ie. 1 byte).

### Unit Testing
Run `./unit_test.sh`. No output indicates the executable has perform successfully. Output indicates that a difference exists between the original file and the encrypted then decrypted file.