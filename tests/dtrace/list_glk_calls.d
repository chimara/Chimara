pid$target:libchimara:glk*:entry
/probefunc != "glk_tick" && stackdepth == 0/
{
	printf("%d %s(0x%X, 0x%X, 0x%X)\t\t\n",stackdepth,probefunc,arg0,arg1,arg2);
}
