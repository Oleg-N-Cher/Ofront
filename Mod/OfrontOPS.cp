MODULE OfrontOPS;	(* NW, RC 6.3.89 / 18.10.92 *)		(* object model 3.6.92 *)
(*
2002-04-11  jt: Number changed for BlackBox 1.4: SHORT(-e)
*)

	IMPORT OPM := OfrontOPM;

	CONST
		MaxIdLen = 48;

	TYPE
		Name* = ARRAY MaxIdLen OF SHORTCHAR;
		String* = POINTER TO ARRAY OF SHORTCHAR;

	(* name, str, numtyp, intval, realval, lrlval are implicit results of Get *)

	VAR
		name-: Name;
		str-: String;
		lstr-: POINTER TO ARRAY OF CHAR;
		numtyp-: SHORTINT; (* 1 = char, 2 = integer, 3 = real, 4 = longreal *)
		intval-: LONGINT;	(* integer value or string length (incl. 0X) *)
		realval-: SHORTREAL;
		lrlval-: REAL;
		lowcase-: BOOLEAN;

	(*symbols:
	    |  0          1          2          3          4
	 ---|--------------------------------------------------------
	  0 |  null       *          /          DIV        MOD
	  5 |  DIV0       REM0       &          +          -
	 10 |  OR         =          #          <          <=
	 15 |  >          >=         IN         IS         ^
	 20 |  .          ,          :          ..         )
	 25 |  ]          }          OF         THEN       DO
	 30 |  TO         BY         (          [          {
	 35 |  ~          :=         number     NIL        string
	 40 |  ident      ;          |          END        ELSE
	 45 |  ELSIF      UNTIL      IF         CASE       WHILE
	 50 |  REPEAT     FOR        LOOP       WITH       EXIT
	 55 |  RETURN     ARRAY      RECORD     POINTER    BEGIN
	 60 |  CONST      TYPE       VAR        PROCEDURE  IMPORT
	 65 |  MODULE     OUT        $          @          eof    *)

	CONST
		(* numtyp values *)
		char = 1; integer = 2; real = 3; longreal = 4;

		(*symbol values*)
		null = 0; times = 1; slash = 2; div = 3; mod = 4;
		div0 = 5; rem0 = 6; and = 7; plus = 8; minus = 9;
		or = 10; eql = 11; neq = 12; lss = 13; leq = 14;
		gtr = 15; geq = 16; in = 17; is = 18; arrow = 19;
		period = 20; comma = 21; colon = 22; upto = 23; rparen = 24;
		rbrak = 25; rbrace = 26; of = 27; then = 28; do = 29;
		to = 30; by = 31; lparen = 32; lbrak = 33; lbrace = 34;
		not = 35; becomes = 36; number = 37; nil = 38; string = 39;
		ident = 40; semicolon = 41; bar = 42; end = 43; else = 44;
		elsif = 45; until = 46; if = 47; case = 48; while = 49;
		repeat = 50; for = 51; loop = 52; with = 53; exit = 54;
		return = 55; array = 56; record = 57; pointer = 58; begin = 59;
		const = 60; type = 61; var = 62; procedure = 63; import = 64;
		module = 65; out = 66; dollar = 67; close = 68;
		at = 69; raw = 70; eof = 71;

	VAR
		ch: CHAR;     (*current character*)
		checkKeyword: PROCEDURE (VAR sym: BYTE);

	PROCEDURE err(n: SHORTINT);
	BEGIN OPM.err(n)
	END err;

	PROCEDURE Str(VAR sym: BYTE);
		VAR i: INTEGER; och, lch: CHAR; long: BOOLEAN;
			s: ARRAY 256 OF CHAR; t: POINTER TO ARRAY OF CHAR;
	BEGIN i := 0; och := ch; long := FALSE;
		LOOP OPM.Get(lch);
			IF lch = och THEN EXIT END;
			IF lch < " " THEN err(3); EXIT END;
			IF lch > 0FFX THEN long := TRUE END;
			IF i < LEN(s) - 1 THEN s[i] := lch
			ELSIF i = LEN(s) - 1 THEN s[i] := 0X; NEW(lstr, 2 * LEN(s)); lstr^ := s$; lstr[i] := lch
			ELSIF i < LEN(lstr^) - 1 THEN lstr[i] := lch
			ELSE t := lstr; t[i] := 0X; NEW(lstr, 2 * LEN(t^)); lstr^ := t^$; lstr[i] := lch
			END;
			INC(i)
		END;
		IF i = 1 THEN sym := number; numtyp := 1; intval := ORD(s[0]);
			IF s[0] < " " THEN err(3) END
		ELSE
			sym := string; numtyp := 0; intval := i + 1; NEW(str, intval);
			IF long THEN
				IF i < LEN(s) THEN s[i] := 0X; NEW(lstr, intval); lstr^ := s$
				ELSE lstr[i] := 0X
				END;
				str^ := SHORT(lstr$)
			ELSE
				IF i < LEN(s) THEN s[i] := 0X; str^ := SHORT(s$);
				ELSE lstr[i] := 0X; str^ := SHORT(lstr$)
				END;
				lstr := NIL
			END
		END;
		OPM.Get(ch)
	END Str;

	PROCEDURE StdKeywords(VAR sym: BYTE);
	BEGIN
		CASE name[0] OF
		| "A":
				IF name = "ARRAY" THEN sym := array END
		| "B":
				IF name = "BEGIN" THEN sym := begin
				ELSIF name = "BY" THEN sym := by
				END
		| "C":
				IF name = "CASE" THEN sym := case
				ELSIF name = "CONST" THEN sym := const
				ELSIF ((OPM.Lang = "C") OR (OPM.Lang = "3")) & (name = "CLOSE") THEN sym := close
				END
		| "D":
				IF name = "DO" THEN sym := do
				ELSIF name = "DIV" THEN sym := div
				ELSIF ((OPM.Lang = "C") OR (OPM.Lang = "3")) & (name = "DIV0") THEN sym := div0
				END
		| "E":
				IF name = "END" THEN sym := end
				ELSIF name = "ELSE" THEN sym := else
				ELSIF name = "ELSIF" THEN sym := elsif
				ELSIF (OPM.Lang # "7") & (name = "EXIT") THEN sym := exit
				END
		| "F":
				IF (OPM.Lang # "1") & (name = "FOR") THEN sym := for END
		| "I":
				IF name = "IF" THEN sym := if
				ELSIF name = "IN" THEN sym := in
				ELSIF name = "IS" THEN sym := is
				ELSIF name = "IMPORT" THEN sym := import
				END
		| "L":
				IF (OPM.Lang # "7") & (name = "LOOP") THEN sym := loop END
		| "M":
				IF name = "MOD" THEN sym := mod
				ELSIF name = "MODULE" THEN sym := module
				END
		| "N":
				IF name = "NIL" THEN sym := nil END
		| "O":
				IF name = "OR" THEN sym := or
				ELSIF name = "OF" THEN sym := of
				ELSIF (OPM.Lang = "C") & (name = "OUT") THEN sym := out
				END
		| "P":
				IF name = "PROCEDURE" THEN sym := procedure
				ELSIF name = "POINTER" THEN sym := pointer
				END
		| "R":
				IF name = "RECORD" THEN sym := record
				ELSIF ((OPM.Lang = "C") OR (OPM.Lang = "3")) & (name = "REM0") THEN sym := rem0
				ELSIF name = "REPEAT" THEN sym := repeat
				ELSIF name = "RETURN" THEN sym := return
				END
		| "T":
				IF name = "THEN" THEN sym := then
				ELSIF name = "TO" THEN sym := to
				ELSIF name = "TYPE" THEN sym := type
				END
		| "U":
				IF name = "UNTIL" THEN sym := until END
		| "V":
				IF name = "VAR" THEN sym := var END
		| "W":
				IF name = "WHILE" THEN sym := while
				ELSIF (OPM.Lang # "7") & (name = "WITH") THEN sym := with
				END
		ELSE
		END
	END StdKeywords;

	PROCEDURE Cmp(IN std, low: ARRAY OF SHORTCHAR): BOOLEAN;
	BEGIN
		IF name = std THEN OPM.Mark(214, OPM.curpos - 1) END;
		RETURN name = low
	END Cmp;

	PROCEDURE LowKeywords(VAR sym: BYTE);
	BEGIN
		CASE CAP(name[0]) OF
		| "A":
				IF Cmp("ARRAY", "array") THEN sym := array END
		| "B":
				IF Cmp("BEGIN", "begin") THEN sym := begin
				ELSIF Cmp("BY", "by") THEN sym := by
				END
		| "C":
				IF Cmp("CASE", "case") THEN sym := case
				ELSIF Cmp("CONST", "const") THEN sym := const
				ELSIF ((OPM.Lang = "C") OR (OPM.Lang = "3")) & Cmp("CLOSE", "close") THEN sym := close
				END
		| "D":
				IF Cmp("DO", "do") THEN sym := do
				ELSIF Cmp("DIV", "div") THEN sym := div
				ELSIF ((OPM.Lang = "C") OR (OPM.Lang = "3")) & Cmp("DIV0", "div0") THEN sym := div0
				END
		| "E":
				IF Cmp("END", "end") THEN sym := end
				ELSIF Cmp("ELSE", "else") THEN sym := else
				ELSIF Cmp("ELSIF", "elsif") THEN sym := elsif
				ELSIF (OPM.Lang # "7") & Cmp("EXIT", "exit") THEN sym := exit
				END
		| "F":
				IF (OPM.Lang # "1") & Cmp("FOR", "for") THEN sym := for END
		| "I":
				IF Cmp("IF", "if") THEN sym := if
				ELSIF Cmp("IN", "in") THEN sym := in
				ELSIF Cmp("IS", "is") THEN sym := is
				ELSIF Cmp("IMPORT", "import") THEN sym := import
				END
		| "L":
				IF (OPM.Lang # "7") & Cmp("LOOP", "loop") THEN sym := loop END
		| "M":
				IF Cmp("MOD", "mod") THEN sym := mod
				ELSIF Cmp("MODULE", "module") THEN sym := module
				END
		| "N":
				IF Cmp("NIL", "nil") THEN sym := nil END
		| "O":
				IF Cmp("OR", "or") THEN sym := or
				ELSIF Cmp("OF", "of") THEN sym := of
				ELSIF (OPM.Lang = "C") & Cmp("OUT", "out") THEN sym := out
				END
		| "P":
				IF Cmp("PROCEDURE", "procedure") THEN sym := procedure
				ELSIF Cmp("POINTER", "pointer") THEN sym := pointer
				END
		| "R":
				IF Cmp("RECORD", "record") THEN sym := record
				ELSIF ((OPM.Lang = "C") OR (OPM.Lang = "3")) & Cmp("REM0", "rem0") THEN sym := rem0
				ELSIF Cmp("REPEAT", "repeat") THEN sym := repeat
				ELSIF Cmp("RETURN", "return") THEN sym := return
				END
		| "S":
				IF name = "system" THEN name := "SYSTEM"
				ELSIF name = "SYSTEM" THEN name := "system"
				END
		| "T":
				IF Cmp("THEN", "then") THEN sym := then
				ELSIF Cmp("TO", "to") THEN sym := to
				ELSIF Cmp("TYPE", "type") THEN sym := type
				END
		| "U":
				IF Cmp("UNTIL", "until") THEN sym := until END
		| "V":
				IF Cmp("VAR", "var") THEN sym := var END
		| "W":
				IF Cmp("WHILE", "while") THEN sym := while
				ELSIF Cmp("WITH", "with") THEN sym := with
				END
		ELSE
		END
	END LowKeywords;

	PROCEDURE AutoKeywords(VAR sym: BYTE);
	BEGIN
		IF name = "MODULE" THEN
			sym := module;
			checkKeyword := StdKeywords;
			lowcase := FALSE
		ELSIF (OPM.Lang = "3") & (name = "module") THEN
			sym := module;
			checkKeyword := LowKeywords;
			lowcase := TRUE
		END
	END AutoKeywords;

	PROCEDURE Identifier(VAR sym: BYTE);
		VAR i: INTEGER;
	BEGIN i := 0;
		REPEAT
			name[i] := SHORT(ch); INC(i); OPM.Get(ch)
		UNTIL (ch < "0") OR ("9" < ch) & (CAP(ch) < "A") OR ("Z" < CAP(ch)) & (ch # "_") OR (i = MaxIdLen);
		IF i = MaxIdLen THEN err(240); DEC(i) END;
		name[i] := 0X; sym := ident;
		checkKeyword(sym)
	END Identifier;

	PROCEDURE Number;
		VAR i, j, m, n, d, e: INTEGER; f, g: REAL; expCh: SHORTCHAR; neg: BOOLEAN;
			dig: ARRAY 24 OF SHORTCHAR;

		PROCEDURE Ord(ch: CHAR; hex: BOOLEAN): INTEGER;
		BEGIN (* ("0" <= ch) & (ch <= "9") OR ("A" <= ch) & (ch <= "F") *)
			IF ch <= "9" THEN RETURN ORD(ch) - ORD("0")
			ELSIF hex THEN RETURN ORD(ch) - ORD("A") + 10
			END; err(2); RETURN 0
		END Ord;

	BEGIN (* ("0" <= ch) & (ch <= "9") *)
		i := 0; m := 0; n := 0; d := 0;
		LOOP (* read mantissa *)
			IF ("0" <= ch) & (ch <= "9") OR (d = 0) & ("A" <= ch) & (ch <= "F") THEN
				IF (m > 0) OR (ch # "0") THEN (* ignore leading zeros *)
					IF n < LEN(dig) THEN dig[n] := SHORT(ch); INC(n) END;
					INC(m)
				END;
				OPM.Get(ch); INC(i)
			ELSIF ch = "." THEN OPM.Get(ch);
				IF ch = "." THEN (* ellipsis *) ch := 7FX; EXIT
				ELSIF d = 0 THEN (* i > 0 *) d := i
				ELSE err(2)
				END
			ELSE EXIT
			END
		END; (* 0 <= n <= m <= i, 0 <= d <= i *)
		IF d = 0 THEN (* integer *)
			IF n = m THEN intval := 0; i := 0;
				IF ch = "X" THEN (* character *) OPM.Get(ch); numtyp := char;
					IF n <= 4 THEN
						WHILE i < n DO intval := intval*10H + Ord(dig[i], TRUE); INC(i) END
					ELSE err(203)
					END
				ELSIF ch = "H" THEN (* hexadecimal *) OPM.Get(ch); numtyp := integer;
					IF (OPM.Lang = "3") OR (OPM.Lang = "7") THEN m := 2*SIZE(LONGINT)
					ELSE m := 2*SIZE(INTEGER)
					END;
					IF n <= m THEN
						IF (n = m) & (dig[0] > "7") THEN (* prevent overflow *) intval := -1;
							IF (OPM.Lang = "3") OR (OPM.Lang = "7") THEN err(203) END
						END;
						WHILE i < n DO intval := intval*10H + Ord(dig[i], TRUE); INC(i) END;
						IF (OPM.Lang = "7") & (intval > MAX(INTEGER)) THEN err(-203) END
					ELSE err(203); intval := 1 (* to prevent "division by zero" on DIV 10000000000000000H *)
					END
				ELSIF (ch = "L") & (OPM.Lang # "3") & (OPM.Lang # "7") THEN (* long hex *)
					OPM.Get(ch); numtyp := integer;
					IF n <= 2*SIZE(LONGINT) THEN
						IF (n = 2*SIZE(LONGINT)) & (dig[0] > "7") THEN (* prevent overflow *) intval := -1 END;
						WHILE i < n DO intval := intval*10H + Ord(dig[i], TRUE); INC(i) END
					ELSE err(203); intval := 1 (* to prevent "division by zero" on DIV 10000000000000000L *)
					END
				ELSE (* decimal *) numtyp := integer;
					WHILE i < n DO d := Ord(dig[i], FALSE); INC(i);
						IF intval <= (MAX(LONGINT) - d) DIV 10 THEN intval := intval*10 + d
						ELSE err(203)
						END
					END;
					IF (OPM.Lang = "7") & (intval > MAX(INTEGER)) THEN err(-203) END
				END
			ELSE err(203)
			END
		ELSE (* fraction *)
			f := 0; g := 0; e := 0; j := 0; expCh := "E";
			WHILE (j < 15) & (j < n) DO g := g * 10 + Ord(dig[j], FALSE); INC(j) END;	(* !!! *)
			WHILE n > j DO (* 0 <= f < 1 *) DEC(n); f := (Ord(dig[n], FALSE) + f)/10 END;
			IF (ch = "E") OR (ch = "D") & (OPM.Lang <= "3") THEN
				expCh := SHORT(ch); OPM.Get(ch); neg := FALSE;
				IF ch = "-" THEN neg := TRUE; OPM.Get(ch)
				ELSIF ch = "+" THEN OPM.Get(ch)
				END;
				IF ("0" <= ch) & (ch <= "9") THEN
					REPEAT n := Ord(ch, FALSE); OPM.Get(ch);
						IF e <= (MAX(SHORTINT) - n) DIV 10 THEN e := SHORT(e*10 + n)
						ELSE err(203)
						END
					UNTIL (ch < "0") OR ("9" < ch);
					IF neg THEN e := -e END
				ELSE err(2)
				END
			END;
			DEC(e, i-d-m); (* decimal point shift *)
			IF (expCh = "E") & (OPM.Lang # "C") THEN numtyp := real;
				IF (1-38 < e) & (e <= 38) THEN
					IF e < j THEN realval := SHORT((f + g) / OPM.IntPower(10, j-e))
					ELSE realval := SHORT((f + g) * OPM.IntPower(10, e-j))
					END
				ELSE err(203)
				END
			ELSE numtyp := longreal;
				IF e < -308 - 16 THEN
					lrlval := 0.0
				ELSIF e < -308 + 14 THEN
					lrlval := (f + g) / OPM.IntPower(10, j-e-30) / 1.0E15 / 1.0E15
				ELSIF e < j THEN
					lrlval := (f + g) / OPM.IntPower(10, j-e)	(* Ten(j-e) *)
				ELSIF e <= 308 THEN
					lrlval := (f + g) * OPM.IntPower(10, e-j)	(* Ten(e-j) *)
				ELSIF e = 308 + 1 THEN
					lrlval := (f + g) * (OPM.IntPower(10, e-j) / 16);
					IF lrlval <= OPM.MaxReal64 / 16 THEN lrlval := lrlval * 16
					ELSE err(203)
					END
				ELSE err(203)
				END
			END
		END
	END Number;

	PROCEDURE Get*(VAR sym: BYTE);
		VAR s: BYTE;

		PROCEDURE Comment (): BOOLEAN;	(* do not read after end of file *)

			VAR n: INTEGER;

			PROCEDURE EnsureLen (len: INTEGER);
				VAR j: INTEGER; s2: String;
			BEGIN
				IF len > LEN(str^) THEN	(* if overflow then increase size of array s *)
					NEW(s2, LEN(str^) * 2); FOR j := 0 TO n - 1 DO s2^[j] := str^[j] END; str := s2
				END
			END EnsureLen;

		BEGIN str := NIL; OPM.Get(ch);
			IF ch # "@" THEN
				LOOP
					LOOP
						WHILE ch = "(" DO OPM.Get(ch);
							IF ch = "*" THEN
								IF Comment() THEN END
							END
						END;
						IF ch = "*" THEN OPM.Get(ch); EXIT END;
						IF ch = OPM.Eot THEN EXIT END;
						OPM.Get(ch)
					END;
					IF ch = ")" THEN OPM.Get(ch); EXIT END;
					IF ch = OPM.Eot THEN err(5); EXIT END
				END;
				RETURN TRUE
			END;
			n := 0; NEW(str, 1024);
			LOOP
				OPM.Get(ch);
				IF ch = "*" THEN OPM.Get(ch);
					IF ch = ")" THEN OPM.Get(ch); str[n] := 0X; EXIT END;
					EnsureLen(n + 1); str[n] := "*"; INC(n)
				END;
				IF ch = OPM.Eot THEN err(5); str[n] := 0X; EXIT END;
				EnsureLen(n + 1); str[n] := SHORT(ch); INC(n)
			END;
			RETURN FALSE
		END Comment;

	BEGIN
		OPM.errpos := OPM.curpos-1;
		WHILE ch <= " " DO (*ignore control characters*)
			IF ch = OPM.Eot THEN sym := eof; RETURN
			ELSE OPM.Get(ch)
			END
		END;
		CASE ch OF   (* ch > " " *)
			| 22X, 27X  : Str(s)
			| "#"  : s := neq; OPM.Get(ch)
			| "&"  : s := and; OPM.Get(ch)
			| "("  : OPM.Get(ch);
							IF ch = "*" THEN
								IF ~Comment() THEN sym := raw; RETURN END;
								Get(s)
							ELSE s := lparen
							END
			| ")"  : s := rparen; OPM.Get(ch)
			| "*"  : s := times; OPM.Get(ch)
			| "+"  : s := plus; OPM.Get(ch)
			| ","  : s := comma; OPM.Get(ch)
			| "-"  : s := minus; OPM.Get(ch)
			| "."  : OPM.Get(ch);
							 IF ch = "." THEN OPM.Get(ch); s := upto ELSE s := period END
			| "/"  : s := slash; OPM.Get(ch)
			| "0".."9": Number; s := number
			| ":"  : OPM.Get(ch);
							 IF ch = "=" THEN OPM.Get(ch); s := becomes ELSE s := colon END
			| ";"  : s := semicolon; OPM.Get(ch)
			| "<"  : OPM.Get(ch);
							 IF ch = "=" THEN OPM.Get(ch); s := leq ELSE s := lss END
			| "="  : s := eql; OPM.Get(ch)
			| ">"  : OPM.Get(ch);
							 IF ch = "=" THEN OPM.Get(ch); s := geq ELSE s := gtr END
			| "@": s := at; OPM.Get(ch)
			| "A".."Z", "a".."z", "_": Identifier(s)
			| "["  : s := lbrak; OPM.Get(ch)
			| "]"  : s := rbrak; OPM.Get(ch)
			| "^"  : s := arrow; OPM.Get(ch)
			| "$"  : s := dollar; OPM.Get(ch)
			| "{"  : s := lbrace; OPM.Get(ch)
			| "|"  : s := bar; OPM.Get(ch)
			| "}"  : s := rbrace; OPM.Get(ch)
			| "~"  : s := not; OPM.Get(ch)
			| 7FX  : s := upto; OPM.Get(ch)
		ELSE s := null; OPM.Get(ch)
		END;
		sym := s
	END Get;

	PROCEDURE Init*;
	BEGIN
		checkKeyword := AutoKeywords;
		lowcase := FALSE;
		ch := " "
	END Init;

END OfrontOPS.

