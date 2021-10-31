^!s::
;ctrl + shift + S

Loop, read, C:\users\caleb\Projects\CityGenerator\tests\output.txt
{
	SendRaw /
	Sleep 100
	SendRaw %A_LoopReadLine%
	Send {Enter}
	; Sleep 1500
    Sleep 50
}