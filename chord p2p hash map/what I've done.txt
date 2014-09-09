{\rtf1\ansi\ansicpg1252\cocoartf1265\cocoasubrtf210
{\fonttbl\f0\fswiss\fcharset0 Helvetica;}
{\colortbl;\red255\green255\blue255;}
\margl1440\margr1440\vieww19600\viewh9680\viewkind0
\pard\tx720\tx1440\tx2160\tx2880\tx3600\tx4320\tx5040\tx5760\tx6480\tx7200\tx7920\tx8640\pardirnatural

\f0\fs24 \cf0 Chord is a p2p distributed hash table algorithm\
\
Basically, this algorithm want to solve a question: Where is the file I want?\
And it want to solve it without centralized server, which is conceptually easier to approach this problem.\
\
Each node will maintain two table.\
	1. finger table: this table will guide a query to where the answer should be. \
	2. file table: provide a mapping \{file_name: ip_address\}\
So, this algorithm is not a distributed storage algorithm. What is distributed is the metadata. Where is the data, who has it, what is his/her IP address? Because it orders nodes into a circle, and the way it design the finger table, every query should be routed to where the answer should be. Also, by adding some redundancy, this algorithm could take care of node failures.}