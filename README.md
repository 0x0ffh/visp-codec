# OCTRA Codec (Vector Integer State Prediction)
Simple cross platform lossless data codec. 
This code is an example of using a data compression method based on orthogonalization of vectors in its simplest implementation.
### Build
```sh
$ g++ main.cpp -o visp.out
```
### How to use
To compress:
```sh
$ ./visp.out c test.txt out.arc
```
To decompress:
```sh
$ ./visp.out d out.arc test.txt
```
