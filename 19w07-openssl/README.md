https://gist.github.com/Soarez/9688998
https://medium.freecodecamp.org/openssl-command-cheatsheet-b441be1e8c4a
https://jamielinux.com/docs/openssl-certificate-authority/

### Priv

    openssl genrsa -out example-private.key 2048

    openssl rsa -in example-private.key -text -noout


### CSR
    openssl req -new -key example-private.key -out example.csr -sha256

    openssl req -in example.csr -text -noout

### CA root (self-signed)

    openssl req -x509 -sha256 -days 365 -newkey rsa:2048 -nodes -keyout ca-root.key -out ca-root.cert

    openssl x509 -in ca-root.cert -text -noout

    openssl verify ca-root.cert

### Sign server


    openssl x509 -req -in example.csr -CA ca-root.cert -CAkey ca-root.key -CAcreateserial -out example.cert

    openssl verify -verbose -CAfile ca-root.cert alice.cert


### TLS handshake

https://www.ibm.com/support/knowledgecenter/en/SSFKSJ_7.1.0/com.ibm.mq.doc/sy10660_.htm    

Check wireshark with the following commands:    

    openssl s_client -host example.com -port 443


https://github.com/CloudFundoo/SSL-TLS-clientserver/blob/master/ssl_server_libssl.c

