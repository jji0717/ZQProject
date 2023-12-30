[ ] // findout what the SilkTest exception functions do 
[ ] 
[ ] // ExceptCalls()
[ ] // ExceptClear()
[ ] // ExceptData()
[ ] // ExceptLog()
[ ] // ExceptNum()
[ ] // ExceptPrint()
[ ] 
[ ] // findout what the results file functions do
[ ] 
[ ] // AppError()
[ ] // FuzzyVerify()
[ ] // LogError()
[ ] // LogVerifyError()
[ ] // LogWarning()
[ ] // RaiseError()
[ ] 
[ ] // AppError()
[ ] // LogError()
[ ] // LogVerifyError()
[ ] // LogWarning()
[ ] // RaiseError()
[ ] 
[ ] // raise
[ ] // reraise
[ ] 
[+] main()
	[ ] // test cases do not fail
	[ ] 
	[ ] tcExceptCalls()
	[ ] tcExceptClear()
	[+] // Results
		[ ] // {}
		[ ] // tcExceptClear CONTINUES 
	[ ] tcExceptData()
	[ ] tcExceptNum()
	[ ] tcExceptPrint()
	[ ] tcLogWarning()
	[ ] tcReRaiseNoFail()
	[ ] 
	[ ] // test cases fail but test case execution continues
	[ ] tcExceptLog()
	[ ] tcLogError()
	[ ] tcLogVerifyError()
	[ ] tcReRaiseFail()
	[ ] 
	[ ] // test cases fail and test case execution does not continue
	[ ] tcAppError()
	[ ] tcRaiseError()
	[ ] tcRaise()
[ ] 
[ ] // test case does not fail
[ ] // just calling ExceptCalls does not log error
[ ] // Results:
[ ] // {{Verify, , 0}, {tcExceptCalls, exceptions.t, 42}, {main, exceptions.t, 21}}
[ ] // tcExceptCalls CONTINUES 
[+] testcase tcExceptCalls()
	[+] do
		[ ] Verify (1, -1, "Error 1 != -1")
	[+] except
		[ ] list of CALL lcCalls
		[ ] lcCalls = ExceptCalls()
		[ ] print (lcCalls)
	[ ] print ("{GetTestCaseName()} CONTINUES {Chr(10)}")
[ ] 
[ ] // test case does not fail
[ ] // ExceptClear clears error info
[ ] // Results
[ ] // {}
[ ] // tcExceptClear CONTINUES 
[+] testcase tcExceptClear()
	[+] do
		[ ] Verify (1, -1, "Error 1 != -1")
	[+] except
		[ ] list of CALL lcCalls
		[ ] ExceptClear()
		[ ] lcCalls = ExceptCalls()
		[ ] print (lcCalls)
	[ ] print ("{GetTestCaseName()} CONTINUES {Chr(10)}")
[ ] 
[ ] // test case does not fail
[ ] // ExceptData returns data associated with the most recent exception
[ ] // Results
[ ] // *** Error: Verify Error 1 != -1 failed - got 1, expected -1
[ ] // tcExceptData CONTINUES 
[+] testcase tcExceptData()
	[+] do
		[ ] Verify (1, -1, "Error 1 != -1")
	[+] except
		[ ] print (ExceptData())
	[ ] print ("{GetTestCaseName()} CONTINUES {Chr(10)}")
[ ] 
[ ] // test case Does fail; but continues
[ ] // ExceptLog
[ ] // Results
[ ] // *** Error: Verify Error 1 != -1 failed - got 1, expected -1
[ ] // Occurred in Verify
[ ] // Called from tcExceptLog at exceptions.t(75)
[ ] // Called from main at exceptions.t(30)
[ ] // tcExceptLog CONTINUES 
[+] testcase tcExceptLog()
	[+] do
		[ ] Verify (1, -1, "Error 1 != -1")
	[+] except
		[ ] ExceptLog()
	[ ] print ("{GetTestCaseName()} CONTINUES {Chr(10)}")
[ ] 
[ ] // test case does not fail
[ ] // ExceptNum returns number of most recent exception
[ ] // Results
[ ] // -13700
[ ] // tcExceptNum CONTINUES 
[+] testcase tcExceptNum()
	[+] do
		[ ] Verify (1, -1, "Error 1 != -1")
	[+] except
		[ ] print (ExceptNum())
	[ ] print ("{GetTestCaseName()} CONTINUES {Chr(10)}")
[ ] 
[ ] // testcase does not fail
[ ] // ExceptPrint print all info about the exception
[ ] // to the results file
[ ] // Results
[ ] // *** Error: Verify Error 1 != -1 failed - got 1, expected -1
[ ] // Occurred in Verify
[ ] // Called from tcExceptPrint at exceptions.t(94)
[ ] // Called from main at exceptions.t(25)
[ ] // tcExceptPrint CONTINUES 
[+] testcase tcExceptPrint()
	[+] do
		[ ] Verify (1, -1, "Error 1 != -1")
	[+] except
		[ ] ExceptPrint()
	[ ] print ("{GetTestCaseName()} CONTINUES {Chr(10)}")
[ ] 
[ ] // halts execution of testcase
[ ] // no Call stack or error number reported
[ ] // Results
[ ] // No calls or error num information reported
[ ] // *** Error: tcAppError should not continue
[ ] // Occurred in AppError
[ ] // Called from tcAppError at exceptions.t(115)
[ ] // Called from main at exceptions.t(31)
[+] testcase tcAppError()
	[+] do
		[ ] Verify (1, -1, "Error 1 != -1")
	[+] except
		[ ] Print("No calls or error num information reported")
		[ ] AppError("{GetTestCaseName()} should not continue")
	[ ] print ("{GetTestCaseName()} CONTINUES {Chr(10)}")
	[ ] 
[ ] 
[ ] // Logs Error message to results file
[ ] // test case DOES fail
[ ] // Results
[ ] // User Generated Message
[ ] // tcLogError CONTINUES 
[+] testcase tcLogError()
	[ ] LogError("User Generated Message")
	[ ] print ("{GetTestCaseName()} CONTINUES {Chr(10)}")
	[ ] 
[ ] 
[ ] // Logs Error message to results file
[ ] // test case DOES fail
[ ] // Results
[ ] // *** Error: Verify User Generated Message failed - got 1, expected -1
[ ] // tcLogVerifyError CONTINUES 
[+] testcase tcLogVerifyError()
	[ ] LogVerifyError(1, -1,"User Generated Message")
	[ ] print ("{GetTestCaseName()} CONTINUES {Chr(10)}")
[ ] 
[ ] // Logs warning message to results file
[ ] // test case does not fail
[ ] // Results
[ ] // User Generated Message
[ ] // tcLogWarning CONTINUES 
[+] testcase tcLogWarning()
	[ ] LogWarning("User Generated Message")
	[ ] print ("{GetTestCaseName()} CONTINUES {Chr(10)}")
[ ] 
[ ] // test case fails
[ ] // Results
[ ] // *** Error: User Generated Message
[ ] // Occurred in RaiseError
[ ] // Called from tcRaiseError at exceptions.t(141)
[ ] // Called from main at exceptions.t(34)
[+] testcase tcRaiseError()
	[+] do
		[ ] Verify(1, -1)
	[+] except
		[ ] RaiseError(22, "User Generated Message")
	[ ] print ("{GetTestCaseName()} CONTINUES {Chr(10)}")
[ ] 
[ ] // Results
[ ] // User Generated Message
[ ] // Occurred in tcRaise at exceptions.t(145)
[ ] // Called from main at exceptions.t(35)
[+] testcase tcRaise()
	[ ] raise 33, "User Generated Message"
	[ ] print ("{GetTestCaseName()} CONTINUES {Chr(10)}")
[ ] 
[ ] // Call a function which raises the expection to 
[ ] // this test case
[ ] // Handle with ExceptPrint(), testcase does not fail
[ ] 
[ ] // Results
[ ] // *** Error: Verify value failed - got 1, expected -1
[ ] // Occurred in Verify
[ ] // Called from fReRaise at exceptions.t(172)
[ ] // Called from tcReRaiseNoFail at exceptions.t(154)
[ ] // Called from main at exceptions.t(27)
[ ] // tcReRaiseNoFail CONTINUES 
[+] testcase tcReRaiseNoFail()
	[+] do
		[ ] fReRaise()
	[+] except
		[ ] ExceptPrint()
	[ ] print ("{GetTestCaseName()} CONTINUES {Chr(10)}")
[ ] 
[ ] // Call a function which raises the expection to 
[ ] // this test case
[ ] // Handle with ExceptLog() and testcase fails
[ ] // Results
[ ] // *** Error: Verify value failed - got 1, expected -1
[ ] // Occurred in Verify
[ ] // Called from fReRaise at exceptions.t(172)
[ ] // Called from tcReRaiseFail at exceptions.t(165)
[ ] // Called from main at exceptions.t(36)
[ ] // tcReRaiseFail CONTINUES 
[+] testcase tcReRaiseFail()
	[+] do
		[ ] fReRaise()
	[+] except
		[ ] ExceptLog ()
	[ ] print ("{GetTestCaseName()} CONTINUES {Chr(10)}")
[ ] 
[+] fReRaise()
	[+] do
		[ ] Verify(1, -1)
	[+] except
		[ ] reraise
