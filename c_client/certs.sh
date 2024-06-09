openssl genpkey -algorithm RSA -out key.pem
openssl req -new -key key.pem -x509 -out cert.pem -days 365
