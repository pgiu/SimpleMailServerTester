# Simple Mail Server Simulator

Pablo Giudice
2016


The aim of this project is to simulate a very simple and linear mail server. 
All the strings are hardcoded and the input is validated against this values. 

The goal is to use this as a testing tool for the simple SMTP client. 

# How to build 

# How to run 

# Expected sequence 

	SRV: Escape character is '^]'.
	SRV: 220 mx0b-00164701.pphosted.com ESMTP cm-m0046761

	CLI: HELO local.usc.edu

	SRV: 250 mx0b-00164701.pphosted.com Hello mail-mip-gw.usc.edu [128.125.253.76], pleased to meet you

	CLI: MAIL FROM: hachuelb@usc.edu

	SRV: 250 2.1.0 Sender ok

	CLI: RCPT TO: pablogiudice@gmail.com

	SRV: 250 2.1.5 Recipient ok

	CLI: DATA

	SRV: 354 Enter mail, end with "." on a line by itself

	CLI: Dear Reader,
	CLI: This is a sample email without subject.
	CLI: Thanks for your patience
	CLI: .

	SRV: 250 2.0.0 25nc6w82mk-1 Message accepted for delivery
	
	CLI: QUIT

	SRV: 221 2.0.0 mx0b-00164701.pphosted.com Closing connection
