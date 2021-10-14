^!s::

Loop, read, C:\users\caleb\Projects\CityGenerator\tests\output.txt
{
	SendRaw /
	Sleep 300
	SendRaw %A_LoopReadLine%
	Sleep 100
	Send {Enter}
	Sleep 1200
}