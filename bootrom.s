0000	(22feff) ldx SP 0xfffe
0003	(76) xor A
0004	(0f1800) jmpw 0x0018
0007	(00) nop
0008	(00) nop
0009	(00) nop
; decode_print
000a	(51) pushx B
; store A into B
000b	(79) mov B, A
; restore original A (0x42 on start)
000c	(8c) mov A, B
; load byte from ptr
000d	(e9) movrp C, H
; xor C with A contents
000e	(16) xor C
000f	(9c) mov A, C
0010	(e1) sout
0011	(74) add A
0012	(c8) inx H
0013	(2f0c00) jmpwnz 0x000c
0016	(52) popx B
0017	(0e) ret

; print CHIP9 BOOT...
0018	(418200) ldx H 0x0082
001b	(9042) ldi A 0x42
; call decode_print
001d	(1e0a00) call 0x000a
; draw boot logo
0020	(f0) clrscr
0021	(41a000) ldx H 0x00a0
0024	(2005) ldi B 0x05
0026	(3020) ldi C 0x20
0028	(ec) movrp A, H
0029	(c8) inx H
002a	(f1) draw
002b	(00) nop
002c	(9008) ldi A 0x08
002e	(14) add C
002f	(90ff) ldi A 0xff
0031	(93) addx D
0032	(00) nop
0033	(00) nop
0034	(00) nop
0035	(905b) ldi A 0x5b
0037	(96) cmp C
0038	(3f2800) jmpwn 0x0028
003b	(03) inc B
003c	(903a) ldi A 0x3a
003e	(86) cmp B
003f	(2f2600) jmpwnz 0x0026
; boot logo finish...
0042	(0f4e00) jmpw 0x004e
; print Self-Test Fail., halt
0045	(417000) ldx H 0x0070
0048	(903b) ldi A 0x3b
; call print_encoded
004a	(1e0a00) call 0x000a
004d	(6c) halt
; after logo
004e	(900f) ldi A 0x0f
0050	(210f10) ldx B 0x100f  ; load the test val
0053	(df03) jmpnn 0x03   ; 0x58
0055	(1e4500) call 0x0045 ; MEEK
0058	(0f4c02) jmpw 0x024c
005b	(00) nop
005c	(00) nop
005d	(ac) mov A, D
005e	(ab) mov L, D
005f	(00) nop
0060	(68) setfnh
0061	(5e) ILL
0062	(57) dec L
0063	(5d) cmps L
0064	(16) xor C
0065	(6f5e48) jmpwnh 0x485e
0068	(4f1b74) jmpwnn 0x741b
006b	(7015) ldi L 0x15
006d	(313b00) ldx D 0x003b
0070	(68) setfnh
0071	(5e) ILL
0072	(57) dec L
0073	(5d) cmps L
0074	(16) xor C
0075	(6f5e48) jmpwnh 0x485e
0078	(4f1b7d) jmpwnn 0x7d1b
007b	(5a) mov D, L
007c	(52) popx B
007d	(57) dec L
007e	(15) and C
007f	(313b00) ldx D 0x003b
0082	(01) ILL
0083	(0a) mov D, B
0084	(0b) mov H, B
0085	(12) ILL
0086	(7b) mov H, A
0087	(62) popx D
0088	(00) nop
0089	(0d) cmps B
008a	(0d) cmps B
008b	(16) xor C
008c	(62) popx D
008d	(10) ILL
008e	(0d) cmps B
008f	(0f6210) jmpw 0x1062
0092	(07) dec B
0093	(14) add C
0094	(0b) mov H, B
0095	(11) ILL
0096	(0b) mov H, B
0097	(0d) cmps B
0098	(0c) movpr H, B
0099	(62) popx D
009a	(03) inc B
009b	(48) setfnn
009c	(42) ILL
009d	(00) nop
009e	(00) nop
009f	(00) nop
00a0	(00) nop
00a1	(00) nop
00a2	(00) nop
00a3	(38) setfn
00a4	(3c) movpr H, E
00a5	(00) nop
00a6	(00) nop
00a7	(00) nop
00a8	(00) nop
00a9	(00) nop
00aa	(01) ILL
00ab	(f0) clrscr
00ac	(0f8000) jmpw 0x0080
00af	(00) nop
00b0	(00) nop
00b1	(00) nop
00b2	(0fe00f) jmpw 0x0fe0
00b5	(f0) clrscr
00b6	(00) nop
00b7	(00) nop
00b8	(00) nop
00b9	(00) nop
00ba	(1fc007) jmpwz 0x07c0
00bd	(f8) ILL
00be	(00) nop
00bf	(00) nop
00c0	(00) nop
00c1	(00) nop
00c2	(7f8001) jmpwc 0x0180
00c5	(fe00) jmpnc 0x00
00c7	(00) nop
00c8	(00) nop
00c9	(00) nop
00ca	(fe00) jmpnc 0x00
00cc	(00) nop
00cd	(7f0000) jmpwc 0x0000
00d0	(00) nop
00d1	(03) inc B
00d2	(f0) clrscr
00d3	(00) nop
00d4	(00) nop
00d5	(0fc000) jmpw 0x00c0
00d8	(00) nop
00d9	(07) dec B
00da	(e0) sin
00db	(00) nop
00dc	(00) nop
00dd	(07) dec B
00de	(e0) sin
00df	(00) nop
00e0	(00) nop
00e1	(0f807c) jmpw 0x7c80
00e4	(3e) ILL
00e5	(03) inc B
00e6	(f0) clrscr
00e7	(00) nop
00e8	(00) nop
00e9	(1f81fc) jmpwz 0xfc81
00ec	(7f81f8) jmpwc 0xf881
00ef	(00) nop
00f0	(00) nop
00f1	(1e07fc) call 0xfc07
00f4	(7fe078) jmpwc 0x78e0
00f7	(00) nop
00f8	(00) nop
00f9	(3e) ILL
00fa	(0ff00f) jmpw 0x0ff0
00fd	(f0) clrscr
00fe	(7c) movpr H, A
00ff	(00) nop
0100	(00) nop
0101	(7c) movpr H, A
0102	(1fc003) jmpwz 0x03c0
0105	(f8) ILL
0106	(3e) ILL
0107	(00) nop
0108	(00) nop
0109	(78) setfc
010a	(3f8003) jmpwn 0x0380
010d	(fc) mov A, A
010e	(1e0000) call 0x0000
0111	(f0) clrscr
0112	(7f0001) jmpwc 0x0100
0115	(fe0f) jmpnc 0x0f
0117	(00) nop
0118	(00) nop
0119	(f0) clrscr
011a	(ff00) jmpnh 0x00
011c	(00) nop
011d	(ff0f) jmpnh 0x0f
011f	(00) nop
0120	(01) ILL
0121	(e1) sout
0122	(fe00) jmpnc 0x00
0124	(00) nop
0125	(ff87) jmpnh 0x87
0127	(8001) ldip H 0x01
0129	(e1) sout
012a	(fe00) jmpnc 0x00
012c	(807f) ldip H 0x7f
012e	(87) ILL
012f	(8003) ldip H 0x03
0131	(c3) popp H
0132	(fc) mov A, A
0133	(01) ILL
0134	(c0) pushp H
0135	(3f8380) jmpwn 0x8083
0138	(03) inc B
0139	(c3) popp H
013a	(fc) mov A, A
013b	(00) nop
013c	(803f) ldip H 0x3f
013e	(c3) popp H
013f	(c0) pushp H
0140	(03) inc B
0141	(87) ILL
0142	(fc) mov A, A
0143	(00) nop
0144	(00) nop
0145	(0fc3c0) jmpw 0xc0c3
0148	(03) inc B
0149	(87) ILL
014a	(fc) mov A, A
014b	(00) nop
014c	(00) nop
014d	(03) inc B
014e	(e1) sout
014f	(c0) pushp H
0150	(07) dec B
0151	(87) ILL
0152	(fc) mov A, A
0153	(00) nop
0154	(01) ILL
0155	(07) dec B
0156	(e1) sout
0157	(e0) sin
0158	(07) dec B
0159	(87) ILL
015a	(fc) mov A, A
015b	(00) nop
015c	(03) inc B
015d	(ffe1) jmpnh 0xe1
015f	(e0) sin
0160	(07) dec B
0161	(87) ILL
0162	(fc) mov A, A
0163	(00) nop
0164	(07) dec B
0165	(ffe1) jmpnh 0xe1
0167	(e0) sin
0168	(07) dec B
0169	(87) ILL
016a	(fc) mov A, A
016b	(00) nop
016c	(0fffe1) jmpw 0xe1ff
016f	(e0) sin
0170	(07) dec B
0171	(87) ILL
0172	(fc) mov A, A
0173	(00) nop
0174	(3fffe1) jmpwn 0xe1ff
0177	(e0) sin
0178	(07) dec B
0179	(87) ILL
017a	(fc) mov A, A
017b	(00) nop
017c	(ffff) jmpnh 0xff
017e	(e1) sout
017f	(e0) sin
0180	(07) dec B
0181	(87) ILL
0182	(fc) mov A, A
0183	(00) nop
0184	(ffff) jmpnh 0xff
0186	(e1) sout
0187	(e0) sin
0188	(07) dec B
0189	(87) ILL
018a	(fc) mov A, A
018b	(01) ILL
018c	(ffff) jmpnh 0xff
018e	(e1) sout
018f	(e0) sin
0190	(07) dec B
0191	(87) ILL
0192	(fc) mov A, A
0193	(03) inc B
0194	(ffff) jmpnh 0xff
0196	(e1) sout
0197	(e0) sin
0198	(07) dec B
0199	(87) ILL
019a	(fc) mov A, A
019b	(03) inc B
019c	(ffff) jmpnh 0xff
019e	(e1) sout
019f	(e0) sin
01a0	(07) dec B
01a1	(87) ILL
01a2	(fc) mov A, A
01a3	(03) inc B
01a4	(ffff) jmpnh 0xff
01a6	(e1) sout
01a7	(e0) sin
01a8	(03) inc B
01a9	(87) ILL
01aa	(fc) mov A, A
01ab	(03) inc B
01ac	(ffff) jmpnh 0xff
01ae	(c3) popp H
01af	(c0) pushp H
01b0	(03) inc B
01b1	(c3) popp H
01b2	(fc) mov A, A
01b3	(03) inc B
01b4	(ffff) jmpnh 0xff
01b6	(c3) popp H
01b7	(c0) pushp H
01b8	(03) inc B
01b9	(c3) popp H
01ba	(fc) mov A, A
01bb	(01) ILL
01bc	(ffff) jmpnh 0xff
01be	(83) addx B
01bf	(8001) ldip H 0x01
01c1	(e1) sout
01c2	(fc) mov A, A
01c3	(01) ILL
01c4	(ffff) jmpnh 0xff
01c6	(87) ILL
01c7	(8001) ldip H 0x01
01c9	(e1) sout
01ca	(fc) mov A, A
01cb	(00) nop
01cc	(ffff) jmpnh 0xff
01ce	(87) ILL
01cf	(8001) ldip H 0x01
01d1	(e0) sin
01d2	(fc) mov A, A
01d3	(00) nop
01d4	(ffff) jmpnh 0xff
01d6	(0f8000) jmpw 0x0080
01d9	(f0) clrscr
01da	(7c) movpr H, A
01db	(00) nop
01dc	(fffe) jmpnh 0xfe
01de	(0f0000) jmpw 0x0000
01e1	(78) setfc
01e2	(3c) movpr H, E
01e3	(00) nop
01e4	(7ffc1e) jmpwc 0x1efc
01e7	(00) nop
01e8	(00) nop
01e9	(7c) movpr H, A
01ea	(1e007f) call 0x7f00
01ed	(f8) ILL
01ee	(3e) ILL
01ef	(00) nop
01f0	(00) nop
01f1	(3c) movpr H, E
01f2	(0e) ret
01f3	(00) nop
01f4	(3ff03c) jmpwn 0x3cf0
01f7	(00) nop
01f8	(00) nop
01f9	(1e0600) call 0x0006
01fc	(3fe078) jmpwn 0x78e0
01ff	(00) nop
0200	(00) nop
0201	(1f0200) jmpwz 0x0002
0204	(1fc0f8) jmpwz 0xf8c0
0207	(00) nop
0208	(00) nop
0209	(0fc000) jmpw 0x00c0
020c	(0e) ret
020d	(03) inc B
020e	(f0) clrscr
020f	(00) nop
0210	(00) nop
0211	(07) dec B
0212	(e0) sin
0213	(00) nop
0214	(00) nop
0215	(03) inc B
0216	(e0) sin
0217	(00) nop
0218	(00) nop
0219	(03) inc B
021a	(f0) clrscr
021b	(00) nop
021c	(00) nop
021d	(0fc000) jmpw 0x00c0
0220	(00) nop
0221	(01) ILL
0222	(fc) mov A, A
0223	(00) nop
0224	(00) nop
0225	(3f8000) jmpwn 0x0080
0228	(00) nop
0229	(00) nop
022a	(7e) ILL
022b	(00) nop
022c	(01) ILL
022d	(fe00) jmpnc 0x00
022f	(00) nop
0230	(00) nop
0231	(00) nop
0232	(3c) movpr H, E
0233	(00) nop
0234	(01) ILL
0235	(fc) mov A, A
0236	(00) nop
0237	(00) nop
0238	(00) nop
0239	(00) nop
023a	(0c) movpr H, B
023b	(00) nop
023c	(00) nop
023d	(f0) clrscr
023e	(00) nop
023f	(00) nop
0240	(00) nop
0241	(00) nop
0242	(00) nop
0243	(00) nop
0244	(00) nop
0245	(c0) pushp H
0246	(00) nop
0247	(00) nop
0248	(00) nop
0249	(00) nop
024a	(00) nop
024b	(00) nop
; jump from self-test check
; print '\a'
024c	(415d00) ldx H 0x005d
024f	(90ab) ldi A 0xab
; call print_encoded
0251	(1e0a00) call 0x000a
; self-check continued
0254	(900f) ldi A 0x0f
0256	(96) cmp C
0257	(af03) jmpz 0x03 ; jump to 0x25c
; MEEK
0259	(1e4500) call 0x0045
; self-check passed
025c	(91) push C
025d	(91) push C
025e	(3022) ldi C 0x22
0260	(92) pop C
0261	(00) nop
0262	(96) cmp C
; if C == A, jump to trailing NOP of 0x268
0263	(af03) jmpz 0x03
0265	(00) nop
0266	(1e4500) call 0x0045
0269	(00) nop
026a	(1f9702) jmpwz 0x0297
026d	(f8) ILL
026e	(eefd) jmpc 0xfd
0270	(e1) sout
0271	(e6) cmpp H
0272	(e1) sout
0273	(e8) ILL
0274	(8e) ILL
0275	(8ffcd6) jmpwnc 0xd6fc
0278	(dc) mov A, L
0279	(db) mov L, L
027a	(ca) mov E, H
027b	(c2) pop H
027c	(8fc2c6) jmpwnc 0xc6c2
027f	(c8) inx H
0280	(c7db) andi A 0xdb
0282	(8fdcdb) jmpwnc 0xdbdc
0285	(c6) cmp H
0286	(c3) popp H
0287	(c3) popp H
0288	(8fcdca) jmpwnc 0xcacd
028b	(8fdac1) jmpwnc 0xc1da
028e	(dc) mov A, L
028f	(db) mov L, L
0290	(ce) ILL
0291	(cd) ILL
0292	(c3) popp H
0293	(ca) mov E, H
0294	(81) push B
0295	(a5) or D
0296	(af90) jmpz 0x90
; 0297: self-test passed (901c) ldi A 0x1c
0298	(1c) movpr H, C
0299	(00) nop
029a	(00) nop
029b	(94) sub C  ; a:1c c:0f
029c	(cf09) jmpn 0x09   ; jmp to 0x2a7
; call print_encoded
; bit n not set -> MEEK
029e	(1e0a00) call 0x000a
02a1	(1e4500) call 0x0045
02a4	(0f9e02) jmpw 0x029e
; continue...
02a7	(00) nop
02a8	(00) nop
02a9	(a702) addi A 0x02
02ab	(b7fd) subi A 0xfd
02ad	(e721) xori A 0x21
02af	(f700) cmpi A 0x00
02b1	(bfef) jmpnz 0xef
02b3	(00) nop
02b4	(00) nop
02b5	(00) nop
02b6	(d70f) ori A 0x0f
02b8	(5001) ldi E 0x01
02ba	(00) nop
02bb	(34) add E
02bc	(ef03) jmph 0x03
02be	(0f9e02) jmpw 0x029e
02c1	(68) setfnh
02c2	(ef02) jmph 0x02
02c4	(9f06) jmp 0x06
02c6	(1e4500) call 0x0045
02c9	(0fc902) jmpw 0x02c9
02cc	(08) clrf
02cd	(fe03) jmpnc 0x03
02cf	(1e9e02) call 0x029e
02d2	(00) nop
; print Self-Test OK.\n
02d3	(416000) ldx H 0x0060
02d6	(903b) ldi A 0x3b
; call print_encoded
02d8	(1e0a00) call 0x000a
02db	(416d02) ldx H 0x026d
02de	(90af) ldi A 0xaf
; call print_encoded
; print "Warning: system might..."
02e0	(1e0a00) call 0x000a
; self-check finished
02e3	(f0) clrscr
02e4	(2004) ldi B 0x04
02e6	(3000) ldi C 0x00
02e8	(9025) ldi A 0x25
02ea	(f1) draw
02eb	(07) dec B
02ec	(9055) ldi A 0x55
02ee	(f1) draw
02ef	(07) dec B
02f0	(9056) ldi A 0x56
02f2	(f1) draw
02f3	(07) dec B
02f4	(9025) ldi A 0x25
02f6	(f1) draw
02f7	(14) add C
02f8	(cc) mov A, H
02f9	(04) add B
02fa	(43) inc H
02fb	(43) inc H
02fc	(43) inc H
02fd	(4a) mov D, H
02fe	(da) mov E, L
; loop
02ff	(fd) movx H, D
; at this point both DE and HL are set to 0x597 (rom start addr)
0300	(ec) movrp A, H
0301	(ed) movx H, B
; start copying byte from 0x597 to 0x325
0302	(7c) movpr H, A
0303	(a8) inx B
0304	(b8) inx D
0305	(902f) ldi A 0x2f
0307	(86) cmp B
0308	(2fff02) jmpwnz 0x02ff
; ==========
; pc:30b sp:fffc a:2f b:2f c:00 d:31 e:72 h:2e l:ff  fl:Z
;(2e) ILL
;0004    (0f1800) jmpw 0x0018

