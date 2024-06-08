package main

import (
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net"
	"os"
	_ "time"
)

func main() {

	ADDRESS := "localhost:5005"

	conn, err := net.Dial("tcp", ADDRESS)
	if err != nil {
		panic(err)
	}

	buf := make([]byte, 1024)
	readlen, _ := conn.Read(buf)
	fmt.Print(string(buf[:readlen]))

	// hello
	_, err = conn.Write([]byte("hell"))
	if err != nil {
		panic(err)
	}
	_, err = conn.Write([]byte("o.."))
	if err != nil {
		panic(err)
	}

	buf = make([]byte, 1024)
	readlen, _ = conn.Read(buf)
	fmt.Print(string(buf[:readlen]))
	
	////////////////// tls start

	// starttls
	_, err = conn.Write([]byte("starttls.."))
	if err != nil {
		panic(err)
	}

	buf = make([]byte, 1024)
	readlen, _ = conn.Read(buf)
	fmt.Print(string(buf[:readlen]))


	cert, err := ioutil.ReadFile("./cert.pem")
	if err != nil {
		log.Fatalf("client: unable to read cert.pem: %v", err)
	}

	// Create a certificate pool with the server certificate
	certPool := x509.NewCertPool()
	if ok := certPool.AppendCertsFromPEM(cert); !ok {
		log.Fatalf("client: unable to parse cert.pem")
	}

	// Step 3: Create a TLS configuration
	tlsConfig := &tls.Config{
		RootCAs:            certPool,
		ServerName:         "localhost",
		InsecureSkipVerify: true,
	}

	// Step 4: Wrap the TCP connection with TLS
	tlsConn := tls.Client(conn, tlsConfig)
	defer tlsConn.Close()

	// Perform the TLS handshake
	if err := tlsConn.Handshake(); err != nil {
		log.Fatalf("client: TLS handshake failed: %s", err)
	}
	fmt.Println("client: TLS connection established")

	////////////////// tls stop


	// tlsConn := conn

	// inc
	_, err = tlsConn.Write([]byte("inc.."))
	if err != nil {
		panic(err)
	}

	buf = make([]byte, 1024)
	readlen, _ = tlsConn.Read(buf)
	fmt.Print(string(buf[:readlen]))

	// states
	_, err = tlsConn.Write([]byte("states.."))
	if err != nil {
		panic(err)
	}

	buf = make([]byte, 10240)
	readlen, _ = tlsConn.Read(buf)
	fmt.Print(string(buf[:readlen]))

	// text
	_, err = tlsConn.Write([]byte("file.."))
	if err != nil {
		panic(err)
	}

	buf = make([]byte, 1024)
	readlen, _ = tlsConn.Read(buf)
	fmt.Print(string(buf[:readlen]))

	// // write text
	// _, err = tlsConn.Write([]byte("hello my name "))
	// if err != nil {
	// 	panic(err)
	// }
	// time.Sleep(time.Second * 2)
	// _, err = tlsConn.Write([]byte("is parth degama\r\n.\r\n"))
	// if err != nil {
	// 	panic(err)
	// }

	path := "/home/parthka/Projects/c_socket_tls/dummy_files/dummy_file_52.bin"
	fs, err := os.Open(path)
	if err != nil {
		log.Fatal(err)
	}

	for {
		buf := make([]byte, 1024)
		buf_len, err := fs.Read(buf)
		if err != nil {
			if err == io.EOF {
				tlsConn.Write([]byte("\r\n.\r\n"))
				break
			} else {
				log.Fatal(err)
			}
		}
		buf = buf[:buf_len]
		tlsConn.Write([]byte(buf))
	}

	buf = make([]byte, 1024)
	readlen, _ = tlsConn.Read(buf)
	fmt.Print(string(buf[:readlen]))

	// states
	_, err = tlsConn.Write([]byte("states.."))
	if err != nil {
		panic(err)
	}

	buf = make([]byte, 1024)
	readlen, _ = tlsConn.Read(buf)
	fmt.Print(string(buf[:readlen]))

	buf = make([]byte, 1024)
	readlen, _ = tlsConn.Read(buf)
	fmt.Println(readlen, string(buf[:readlen]))

	buf = make([]byte, 1024)
	readlen, _ = tlsConn.Read(buf)
	fmt.Println(readlen, string(buf[:readlen]))

	buf = make([]byte, 1024)
	readlen, _ = tlsConn.Read(buf)
	fmt.Println(readlen, string(buf[:readlen]))

	buf = make([]byte, 1024)
	readlen, _ = tlsConn.Read(buf)
	fmt.Println(readlen, string(buf[:readlen]))

	buf = make([]byte, 1024)
	readlen, _ = tlsConn.Read(buf)
	fmt.Println(readlen, string(buf[:readlen]))

	buf = make([]byte, 1024)
	readlen, _ = tlsConn.Read(buf)
	fmt.Println(readlen, string(buf[:readlen]))

	buf = make([]byte, 1024)
	readlen, _ = tlsConn.Read(buf)
	fmt.Println(readlen, string(buf[:readlen]))

	fmt.Println("quit...")

	conn.Close()
	tlsConn.Close()

}
