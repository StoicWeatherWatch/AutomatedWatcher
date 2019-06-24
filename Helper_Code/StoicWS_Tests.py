# Paste Stoic WS here and run

StoicTest = StoicWatcher("/dev/ttyACM1", 9600, 1)

testResult = StoicTest.parse_2T_BoxTemp("*2T,2AA1")
print testResult
