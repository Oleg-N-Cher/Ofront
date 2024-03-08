MODULE OfrontCmd;	(* J. Templ 3.2.95 *)

	IMPORT
		OPP := OfrontOPP, OPB := OfrontOPB, OPT := OfrontOPT,
		OPV := OfrontOPV, OPC := OfrontOPC, OPM := OfrontOPM;

	PROCEDURE Module*(VAR done: BOOLEAN);
		VAR ext, new, main: BOOLEAN; p: OPT.Node;
	BEGIN
		OPV.Init; OPP.Module(p, OPM.opt);
		IF OPM.noerr THEN
			OPV.AdrAndSize(OPT.topScope);
			main := (OPM.mainprog IN OPM.opt) & (OPM.modName # "SYSTEM");
			IF ~main THEN OPT.Export(ext, new) END;
			IF OPM.noerr THEN
				OPM.OpenFiles(OPT.SelfName);
				OPV.Module(p);
				IF OPM.noerr THEN
					IF main THEN
						OPM.DeleteNewSym; OPM.LogWStr("  main program")
					ELSE
						IF new THEN OPM.LogWStr("  new symbol file"); OPM.RegisterNewSym
						ELSIF ext THEN OPM.LogWStr("  extended symbol file"); OPM.RegisterNewSym
						END
					END
				ELSE OPM.DeleteNewSym
				END
			END
		END;
		OPM.CloseFiles;
		IF OPT.topScope # NIL THEN OPT.Close END; (* sonst Trap nach Fehler *)
		OPM.LogWLn; done := OPM.noerr;
		IF OPM.warning156 THEN OPM.Mark(-156, -1) END;
		IF ~done THEN OPM.InsertMarks END
	END Module;

	PROCEDURE Translate*;
		VAR done: BOOLEAN;
	BEGIN
		OPM.OpenPar(FALSE);
		OPM.Init(done);
		OPM.InitOptions;
		IF done THEN Module(done) END (* Compile source to .c and .h files *)
	END Translate;

	PROCEDURE TranslateDone* (): BOOLEAN;
		VAR done: BOOLEAN;
	BEGIN
		OPM.OpenPar(FALSE);
		OPM.Init(done);
		OPM.InitOptions;
		IF done THEN Module(done) END; (* Compile source to .c and .h files *)
	RETURN done
	END TranslateDone;

	PROCEDURE TranslateModuleList*;
		VAR done: BOOLEAN;
	BEGIN
		OPM.OpenPar(TRUE);
		LOOP
			OPM.Init(done);
			IF ~done THEN EXIT END ;
			OPM.InitOptions;
			Module(done); (* Compile source to .c and .h files *)
			IF ~done THEN EXIT END
		END
	END TranslateModuleList;

BEGIN
	OPB.typSize := OPV.TypSize; OPT.typSize := OPV.TypSize
END OfrontCmd.

DevCompiler.CompileThis
	OfrontOPM OfrontOPS OfrontOPT OfrontOPC OfrontOPB OfrontOPV OfrontOPP
	OfrontCmd OfrontBrowser