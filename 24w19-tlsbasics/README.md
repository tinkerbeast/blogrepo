TLS - Basics
============


OSI stack and handshake
-----------------------

### OSI layers

![tls osi](https://hpbn.co/assets/diagrams/9873c7441be06e0b53a006aac442696c.svg?_sw-precache=9873c7441be06e0b53a006aac442696c)
* [Handshake protocol](https://datatracker.ietf.org/doc/html/rfc5246#section-7)
* [Record protocol](https://datatracker.ietf.org/doc/html/rfc5246#section-6)

Sub-protocols:
* Handshake protocol
* Change cipher spec protocol
* Alert protocol
* Application data protocol

NOTE:
* OSI layer dichotomy (Data + Control).
* OSI layer isn't so strict.
* TLS's position in OSI layer.
* TLS needs to sit on top of a stateful protocol.

### Handshaking

![TLS layers](https://cdn.ttgtmedia.com/rms/onlineimages/security-tls_1.3_handshake-h.png)

* [TCP state machine example](https://en.wikipedia.org/wiki/File:Tcp_state_diagram.png)

### Refs

* [Nice intro from IBM docs](https://www.ibm.com/docs/en/sdk-java-technology/8?topic=handshake-tls-12-protocol)
* [TLS1.3 RFC ref](https://datatracker.ietf.org/doc/html/rfc8446)
* [TLS1.2 RFC ref](https://datatracker.ietf.org/doc/html/rfc5246)

TLS suites and encryption basics
--------------------------------

### TLS wireshark with cipher suite

TODO: See wireshark

### A cipher suite

![cipher suite](https://www.keyfactor.com/wp-content/uploads/CipherSuite1.png)

Terminology: 
* HMAC not MAC. Or Signature.
* Bulk cipher, not just cipher.

### Cipher suites

https://en.wikipedia.org/wiki/Cipher_suite#Supported_algorithms
https://ciphersuite.info/

### TLS 1.3 caveat

![TLS13 suite](https://www.keyfactor.com/wp-content/uploads/CipherSuite2.png)
TODO: Fully explain what happens in case of TLS1.3

### OpenSSL commands

* [OpenSSL3.2 ciphers](https://www.openssl.org/docs/man3.2/man1/ciphers.html)
```
rishin@wri:~$ openssl ciphers -tls1_2 DHE | tr ":" "\n"
TLS_AES_256_GCM_SHA384
TLS_CHACHA20_POLY1305_SHA256
TLS_AES_128_GCM_SHA256
DHE-DSS-AES256-GCM-SHA384
DHE-RSA-AES256-GCM-SHA384
...
```
NOTE:
* Why search? Because OpenSSL names do not match RFC.

### Refs

* https://www.keyfactor.com/blog/cipher-suites-explained/
* [TLS 1.3 ref](https://datatracker.ietf.org/doc/html/rfc8446#appendix-B.4)
* [TLS 1.2 ref](https://datatracker.ietf.org/doc/html/rfc5246#appendix-A.5)


Cryptography basics
-------------------

### Terminology

* Keys - Lock and unlock concept
* Channel - Medium of communication.

Symmetry:
* Symmetric key - Locking and unlocking key are the same.
* Asymmetric key - Unlocking key is different from locking key.

Visibility:
* Public - Data accessible to everyone.
* Private - Data accessible only to you.

Etc.:
* Shared secret - Any data known by **only** the intended party.
* Shared key - Not so well defined.
* Pre-shared key - A _shared secret_ which is also a _key_ which was known before _channel_ establishment.
* Trapdoor functions - Given a known set of x, f(x) is easily computed and f_inverse(x) is computationally complex.

### Modulo math rules

* (x+y) mod m = ((x mod m) + (y mod m)) mod m
* (x-y) mod m = ((x mod m) - (y mod m)) mod m
* (x.y) mod m = ((x mod m) . (y mod m)) mod m
* (x / y) mod m - No simple reduction formula exists. But can be converted to (x . z) mod m form.

[Modular division](https://www.khanacademy.org/computing/computer-science/cryptography/modarithmetic/a/what-is-modular-arithmetic)

### `pow(g, x) mod p` as a trapdoor function

```
def mypow(g, x, p):
    if x == 1 or x == 0: return 1
    out = myypow(g, x/2, p)
    if x % 2 == 0:
        return (out * out) % p
    else:
        return (out * out * g) % p
```

NOTE:
* Modulo function in C, Python, etc.
* Modulo vs modulus.


Key exchange
------------

### Diffie Hellman

Parameters:
* g - Generator
* p - Modulus.

![DH](https://upload.wikimedia.org/wikipedia/commons/c/c8/DiffieHellman.png)
Naming convention:
A,B,C - Alice, Bob, Carol
M - Mallory (malicious)
E - Eve (Evil)
![xkcd-protocol](https://imgs.xkcd.com/comics/protocol.png)

### Diffie-Hellman in OpenSSL
Note - The following commands were run on a Ubuntu14.04 docker image. Because DH is so old, modern TLS versions don't support it.

Server on 172.17.10.3:
```
openssl s_server -ssl3 -nocert -cipher 'ADH-DES-CBC3-SHA'
```
See: https://www.openssl.org/docs/man3.2/man1/s_server.html

Client on 172.17.10.2:
```
openssl s_client -ssl3 -cipher 'ADH-DES-CBC3-SHA' -state -connect 172.17.0.3:4433
```
See: https://www.openssl.org/docs/man3.2/man1/openssl-s_client.html

**TODO: wireshark traces**

Server output:
```
Using default temp DH parameters
Using default temp ECDH parameters
ACCEPT
bad gethostbyaddr
-----BEGIN SSL SESSION PARAMETERS-----
MHUCAQECAgMABAIAGwQg76D6Pd2BDdUuTB+4zYi1+v+G22UCQW9NMgFyj6Q0hLoE
MAHm/fm7iji2XPWr4vmO1bh3UAOMyt5+iESNq9lYDjx6vnD1wi/eLK+Z1MOtj08v
/KEGAgRmONx5ogQCAhwgpAYEBAEAAAA=
-----END SSL SESSION PARAMETERS-----
Shared ciphers:ADH-DES-CBC3-SHA
CIPHER is ADH-DES-CBC3-SHA
Secure Renegotiation IS supported
```

Client output:
```
CONNECTED(00000003)
SSL_connect:before/connect initialization
SSL_connect:SSLv3 write client hello A
SSL_connect:SSLv3 read server hello A
SSL_connect:SSLv3 read server key exchange A
SSL_connect:SSLv3 read server done A
SSL_connect:SSLv3 write client key exchange A
SSL_connect:SSLv3 write change cipher spec A
SSL_connect:SSLv3 write finished A
SSL_connect:SSLv3 flush data
SSL_connect:SSLv3 read finished A
---
no peer certificate available
---
No client certificate CA names sent
---
SSL handshake has read 698 bytes and written 394 bytes
---
New, TLSv1/SSLv3, Cipher is ADH-DES-CBC3-SHA
Secure Renegotiation IS supported
Compression: NONE
Expansion: NONE
SSL-Session:
    Protocol  : SSLv3
    Cipher    : ADH-DES-CBC3-SHA
    Session-ID: EFA0FA3DDD810DD52E4C1FB8CD88B5FAFF86DB6502416F4D3201728FA43484BA
    Session-ID-ctx: 
    Master-Key: 01E6FDF9BB8A38B65CF5ABE2F98ED5B87750038CCADE7E88448DABD9580E3C7ABE70F5C22FDE2CAF99D4C3AD8F4F2FFC
    Key-Arg   : None
    PSK identity: None
    PSK identity hint: None
    SRP username: None
    Start Time: 1715002489
    Timeout   : 7200 (sec)
    Verify return code: 0 (ok)
---
```

### DH parameter generation
Command:
```
openssl dhparam -text 512
```
Output:
```
Generating DH parameters, 512 bit long safe prime
...........+.+.............+...........++*++*++*++*++*++*++*++*++*++*++*++*++*++*++*++*++*++*++*++*++*++*++*++*++*++*++*++*
    DH Parameters: (512 bit)
    P:   
        00:a4:00:5c:04:ea:20:70:2c:61:5e:21:59:7a:f3:
        cb:f5:b2:f3:30:bc:73:84:49:db:85:87:3f:95:ae:
        58:3c:d4:22:e5:09:61:18:1c:d6:c9:71:9d:41:f8:
        0f:f6:18:41:33:02:4b:22:1e:de:02:75:36:fd:f3:
        7d:52:d8:29:27
    G:    2 (0x2)
-----BEGIN DH PARAMETERS-----
MEYCQQCkAFwE6iBwLGFeIVl688v1svMwvHOESduFhz+Vrlg81CLlCWEYHNbJcZ1B
+A/2GEEzAksiHt4CdTb9831S2CknAgEC
-----END DH PARAMETERS-----
```

NOTE:
* PEM format.


Man in the middle
------------

TODO

Appendix 1: History
-------------------

TODO

Appendix 2: Self study
---------------------------

* Convert the TLS handshake to a state machine diagram.
* Research on low computation invertibility `pow(g, x) mod m`.
* Memorise terminology and parameters.
* Open point - Utility of random number passed (replay attacks).
* Advanced - Write a MTM application hack our current client server handshake.


Appendix 3: TODO TLS networking related
--------------------------------------------
* Record fragmentation
* Record compression decompression

Appendix 3: TODO TLS security theoretical
---------------------------------------
* Payload protection
* Key generation


TLS basics 2
==========

RSA maths
-------------
### RSA parameters
```
prime1: p
prime2: q
modulus: n = p * q
publicExponent: e    // encryption key
privateExponent: d   // decryption key
// following parameters are kept for easier computation - can be ignored.
exponent1 = d*p
exponent2 = d*q
coefficient = q**-1 mod p
```
Parameter constraints:
```
e is prime
e*d ≡ 1 mod φ(n)  // e*d = k*φ(n) + 1        ------------ i
e < φ(n) // to avoid bounds cases
φ(n) % e != 0  // to avoid trivial cases
```
NOTE: φ(n) is the totient function. In our case φ(n) = (p-1)*(q-1)

### Euler Fermat theorem
```
If y is a positive integer, and gcd(x,y)=1, then x**φ(y) ≡ 1 mod y   ------ ii
Note that, x**φ(y) = k.y + 1
```
### Encryption decryption
```
M               // to be encrypted
We must make M relatively prime to n   ---------- iii

R = M**e mod n  // is the encrypted message

// How to decrypt
R**d    := (M**e)**d
        := M**(e.d)
        := M**(k*φ(n) + 1)     --------- i
        := M**(k*φ(n)) * M
        := (M**φ(n))**k * M
        := (1 mod n)**k * M    ---------- ii & iii
```

### RSA parameter calculations

Proof that e*d ≡ 1 mod φ(n) can be calculated fast enough:
```
e = (k*φ(n) + 1) / d
This can be solved using -
https://math.stackexchange.com/questions/497327/find-point-on-line-that-has-integer-coordinates
https://www.geeksforgeeks.org/find-integer-point-line-segment-given-two-ends/
```
TODO: Proof that prime numbers can be generated fast enough.

### Summary
>>> Parameter are: modulus: n = p * q,  publicExponent: e, privateExponent: d
>>> If message is M: `M**e` encrypts, `(M**e)**d` decrypts


RSA private key
-------------------
```
openssl genrsa -out example-private.key 2048
openssl rsa -in example-private.key -text -noout
```


MTM revision
-----------------
![alice-bob-eve](https://pajhome.org.uk/crypt/rsa/problem.gif)
>.
>.
>.
>.
>.
>.
>.

### Certificates

```
openssl req -x509 -sha256 -days 365 -newkey rsa:2048 -nodes -keyout ca-root.key -out ca-root.cert
openssl x509 -in ca-root.cert -text -noout
```


Master secret and moving on to bulk encryption
----------------------------------------------------------

### Pre-master Secret, master secret

* Diffie Hellman pre-master secret
* RSA pre-master secret (sent over Handshake/ClientKeyExchange. See https://datatracker.ietf.org/doc/html/rfc5246#section-7.4.7.1)

master secret - A 48-byte secret shared between the two peers in the connection.

[Pre-master secret conversion to master secret](https://datatracker.ietf.org/doc/html/rfc5246#section-8.1)

### Initialization Vector (IV)
TODO

### Change cipher spec protocol
https://datatracker.ietf.org/doc/html/rfc5246#section-7.1


RSA key exchange
----------------------
```
openssl s_server -no_tls1_3 -cert ca-root.cert -key ca-root.key -cipher AES256-SHA256
openssl s_client -no_tls1_3 -cipher AES256-SHA256 -connect 127.0.0.1:4433
```


Adding to the confusion
-----------------------------
```
// TLS_DHE_RSA_WITH_AES_256_GCM_SHA384
openssl s_server -no_tls1_3 -cert ca-root.cert -key ca-root.key -cipher DHE-RSA-AES256-GCM-SHA384
openssl s_client -no_tls1_3 -cipher DHE-RSA-AES256-GCM-SHA384 -connect 127.0.0.1:4433

// TLS_DHE_PSK_WITH_AES_256_GCM_SHA384
openssl s_server -no_tls1_3 -cert ca-root.cert -key ca-root.key -cipher DHE-PSK-AES256-GCM-SHA384 -psk abcd1234
openssl s_client -no_tls1_3 -cipher DHE-PSK-AES256-GCM-SHA384 -connect 127.0.0.1:4433 -psk abcd1234

// TLS_RSA_PSK_WITH_AES_256_GCM_SHA384
openssl s_server -no_tls1_3 -cert ca-root.cert -key ca-root.key -cipher RSA-PSK-AES256-GCM-SHA384 -psk abcd1234
openssl s_client -no_tls1_3 -cipher RSA-PSK-AES256-GCM-SHA384 -connect 127.0.0.1:4433 -psk abcd1234
```

