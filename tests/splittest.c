#include <stdio.h>
#include <string.h>
#include <libchimara/glk.h>

#define SPACE_FACTOR 1.8

void center_text(winid_t win, char *text)
{
	glui32 width, height;
	glk_window_get_size(win, &width, &height);
	
	glk_set_window(win);
	glk_window_clear(win);
	
	if(glk_window_get_type(win) == wintype_TextGrid) {
		glk_window_move_cursor(win, width / 2 - strlen(text) / 2, height / 2);
		glk_put_string(text);
	} else if(glk_window_get_type(win) == wintype_TextBuffer) {
		int count;
		for(count = 0; count < height / 2; count++)
			glk_put_char('\n');
		for(count = 0; 
			count < (int)(SPACE_FACTOR * (width / 2 - strlen(text) / 2)); 
			count++)
			glk_put_char(' ');
		glk_put_string(text);
	}
}

void print_two_rows(winid_t win)
{
	glui32 width, height;
	glk_window_get_size(win, &width, &height);
	
	glk_set_window(win);
	glk_window_clear(win);

	glui32 x = (width >= 6)? width / 2 - 3 : 0;
	glui32 y = (height > 0)? (height - 1) / 2 : 0;

	glk_window_move_cursor(win, x, y);
	glk_put_string("C: 2");
	glk_window_move_cursor(win, x + 3, y + 1);
	glk_put_string("rows");
}

void wait_for_key(winid_t win)
{
    event_t ev;
	glk_request_char_event(win);
	do
		glk_select(&ev);
    while(ev.type != evtype_CharInput);
}

void glk_main(void)
{
    winid_t win_a = NULL, win_b = NULL, win_c = NULL, win_d = NULL;
	
	fprintf(stderr, "TEST CASES FROM GLK SPEC\n\n"
			"(Press a key in window A to continue each time)\n\n"
			"Say you do two splits, each a 50-50 percentage split. You start\n"
			"with the original window A, and split that into A and B; then\n"
			"you split B into B and C.\n\n");
	
	win_a = glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	win_b = glk_window_open(win_a, winmethod_Proportional | winmethod_Below,
							50, wintype_TextBuffer, 0);
	win_c = glk_window_open(win_b, winmethod_Proportional | winmethod_Below,
							50, wintype_TextBuffer, 0);
	if(!win_a || !win_b || !win_c)
		return;
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_c, "C");
	
	wait_for_key(win_a);
	glk_window_close(glk_window_get_root(), NULL);
	
	fprintf(stderr, "Or, you could split A into A and B, and then split A\n"
			"again into A and C.\n\n");
	
	win_a = glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	win_b = glk_window_open(win_a, winmethod_Proportional | winmethod_Below,
							50, wintype_TextBuffer, 0);
	win_c = glk_window_open(win_a, winmethod_Proportional | winmethod_Below,
							50, wintype_TextBuffer, 0);
	if(!win_a || !win_b || !win_c)
		return;
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_c, "C");
	
	wait_for_key(win_a);
	glk_window_close(glk_window_get_root(), NULL);
	
	fprintf(stderr, "Here are more ways to perform the first example; all of\n"
			"them have the same tree structure, but look different on the\n"
			"screen. Here, we turn the second split (B into B/C) upside down;\n"
			"we put the new window (C) above the old window (B).\n\n");
	
	win_a = glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	win_b = glk_window_open(win_a, winmethod_Proportional | winmethod_Below,
							50, wintype_TextBuffer, 0);
	win_c = glk_window_open(win_b, winmethod_Proportional | winmethod_Above,
							50, wintype_TextBuffer, 0);
	if(!win_a || !win_b || !win_c)
		return;
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_c, "C");
	
	wait_for_key(win_a);
	glk_window_close(glk_window_get_root(), NULL);
			
	fprintf(stderr, "Here, we mess with the percentages. The first split (A\n"
			"into A/B) is a 25-75 split, which makes B three times the size\n"
			"of A. The second (B into B/C) is a 33-66 split, which makes C\n"
			"twice the size of B. This looks rather like the second example,\n"
			"but has a different internal structure.\n\n");
	
	win_a = glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	win_b = glk_window_open(win_a, winmethod_Proportional | winmethod_Below,
							75, wintype_TextBuffer, 0);
	win_c = glk_window_open(win_b, winmethod_Proportional | winmethod_Below,
							67, wintype_TextBuffer, 0);
	if(!win_a || !win_b || !win_c)
		return;
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_c, "C");
	
	wait_for_key(win_a);
	glk_window_close(glk_window_get_root(), NULL);
	
	fprintf(stderr, "Here, the second split (B into B/C) is vertical instead\n"
			"of horizontal, with the new window (C) on the left of the old\n"
			"one.\n\n");
	
	win_a = glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	win_b = glk_window_open(win_a, winmethod_Proportional | winmethod_Below,
							50, wintype_TextBuffer, 0);
	win_c = glk_window_open(win_b, winmethod_Proportional | winmethod_Left,
							50, wintype_TextBuffer, 0);
	if(!win_a || !win_b || !win_c)
		return;
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_c, "C");
	
	wait_for_key(win_a);
	glk_window_close(glk_window_get_root(), NULL);
	
	fprintf(stderr, "In the following two-split process, you can see that\n"
			"when a window is split, it is replaced by a new pair window, and\n"
			"moves down to become one of its two children.\n\n");
	
	if(!(win_a = glk_window_open(0, 0, 0, wintype_TextBuffer, 0)))
		return;
	center_text(win_a, "A");
	wait_for_key(win_a);
	
	if(!(win_b = glk_window_open(win_a, 
								 winmethod_Proportional | winmethod_Below,
								 50, wintype_TextBuffer, 0)))
		return;
	center_text(win_a, "A");
	center_text(win_b, "B");
	wait_for_key(win_a);
	
	if(!(win_c = glk_window_open(win_b, winmethod_Proportional | winmethod_Left,
								 50, wintype_TextBuffer, 0)))
		return;
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_c, "C");
	wait_for_key(win_a);
	
	glk_window_close(glk_window_get_root(), NULL);
	
	fprintf(stderr, "What happens when there is a conflict? The rules are\n"
			"simple. Size control always flows down the tree, and the player\n"
			"is at the top. Let's bring out an example: first we split A into\n"
			"A and B, with a 50%% proportional split. Then we split A into A\n"
			"and C, with C above, being a text grid window, and C gets a\n"
			"fixed size of two rows (as measured in its own font size). A\n"
			"gets whatever remains of the 50%% it had before.\n\n");
	
	win_a = glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	win_b = glk_window_open(win_a, winmethod_Proportional | winmethod_Below,
							50, wintype_TextBuffer, 0);
	win_c = glk_window_open(win_a, winmethod_Fixed | winmethod_Above,
							2, wintype_TextGrid, 0);
	if(!win_a || !win_b || !win_c)
		return;
	center_text(win_a, "A");
	center_text(win_b, "B: 50%");
	print_two_rows(win_c);
	wait_for_key(win_a);
	
	fprintf(stderr, "(Stage 1) Now the player stretches the window\n"
			"vertically.\n\n");
	
	wait_for_key(win_a);
	
	fprintf(stderr, "(Stage 2) Then the user maliciously starts squeezing the\n"
			"window down, in stages.\n\n");
	
	center_text(win_a, "A");
	center_text(win_b, "B: 50%");
	print_two_rows(win_c);
	wait_for_key(win_a);
	
	fprintf(stderr, "(Stage 3) The logic remains the same. At stage 3,\n"
			"there's no room left for A, so it winds up with zero height.\n"
			"Nothing displayed in A will be visible.\n\n");
	
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_c, "C");
	wait_for_key(win_a);
	
	fprintf(stderr, "(Stage 4) At stage 4, there isn't even room in the upper\n"
			"50%% to give C its two rows; so it only gets one.\n\n");
	
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_c, "C");
	wait_for_key(win_a);
	
	fprintf(stderr, "(Stage 5) Finally, C is squashed out of existence as\n"
			"well.\n\n");
	
	center_text(win_a, "A");
	center_text(win_b, "B");
	wait_for_key(win_a);
	
	glk_window_close(glk_window_get_root(), NULL);
	
	fprintf(stderr, "What happens when you split a fixed-size window? The\n"
			"resulting pair window retains the same size constraint as the\n"
			"original window that was split. The key window for the original\n"
			"split is still the key window for that split, even though it's\n"
			"now a grandchild instead of a child.\n\n");
	
	if(!(win_a = glk_window_open(0, 0, 0, wintype_TextBuffer, 0)))
		return;
	center_text(win_a, "A");
	wait_for_key(win_a);
	
	fprintf(stderr, "After the first split, the new pair window (O1, which\n"
			"covers the whole screen) knows that its first child (A) is above\n"
			"the second, and gets 50%% of its own area. (A is the key window\n"
			"for this split, but a proportional split doesn't care about key\n"
			"windows.)\n\n");
	
	if(!(win_b = glk_window_open(win_a, 
								 winmethod_Proportional | winmethod_Below,
								 50, wintype_TextBuffer, 0)))
		return;
	center_text(win_a, "A: 50%");
	center_text(win_b, "B");
	wait_for_key(win_a);
	
	fprintf(stderr, "After the second split, all this remains true; O1 knows\n"
			"that its first child gets 50%% of its space, and A is O1's key\n"
			"window. But now O1's first child is O2 instead of A. The newer\n"
			"pair window (O2) knows that its first child (C) is above the\n"
			"second, and gets a fixed size of two rows. (As measured in C's\n"
			"font, because C is O2's key window.)\n\n");
	
	if(!(win_c = glk_window_open(win_a, winmethod_Fixed | winmethod_Above, 2,
								 wintype_TextGrid, 0)))
		return;
	center_text(win_a, "A");
	center_text(win_b, "B");
	print_two_rows(win_c);
	wait_for_key(win_a);
	
	fprintf(stderr, "If we split C, now, the resulting pair will still be two\n"
			"C-font rows high -- that is, tall enough for two lines of\n"
			"whatever font C displays. For the sake of example, we'll do this\n"
			"vertically.\n\n");
	
	if(!(win_d = glk_window_open(win_c, 
								 winmethod_Proportional | winmethod_Right,
								 50, wintype_TextBuffer, 0)))
		return;
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_c, "C");
	center_text(win_d, "D");
	wait_for_key(win_a);
	
	fprintf(stderr, "When you close a window (and it is not the root window),\n"
			"the other window in its pair takes over all the freed-up area.\n"
			"Let's close D, in the current example:\n\n");
	
	glk_window_close(win_d, NULL);
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_c, "C");
	wait_for_key(win_a);
	
	fprintf(stderr, "But what if we had closed C instead of D? We would have\n"
			"gotten this:\n\n");
	
	glk_window_close(glk_window_get_root(), NULL);
	win_a = glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	win_b = glk_window_open(win_a, winmethod_Proportional | winmethod_Below, 50,
							wintype_TextBuffer, 0);
	win_c = glk_window_open(win_a, winmethod_Fixed | winmethod_Above, 2,
							wintype_TextGrid, 0);
	win_d = glk_window_open(win_c, winmethod_Proportional | winmethod_Right, 50,
							wintype_TextBuffer, 0);
	glk_window_close(win_c, NULL);
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_d, "D");
	wait_for_key(win_a);
	
	fprintf(stderr, "Consider the example above, where D had collapsed to\n"
			"zero height. Say D was a text buffer window. You could make a\n"
			"more useful layout:\n\n");
	winid_t o2 = glk_window_get_parent(win_d);
	glk_window_set_arrangement(o2, winmethod_Above | winmethod_Fixed, 3, win_d);
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_d, "D");
	wait_for_key(win_a);
	
	fprintf(stderr, "If you later wanted to expand D to five rows:\n\n");
	glk_window_set_arrangement(o2, winmethod_Above | winmethod_Fixed, 5, NULL);
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_d, "D");
	wait_for_key(win_a);
	
	fprintf(stderr, "This changes the constraint to be on the lower child of\n"
			"O2, which is A. The key window is still D; so A would then be\n"
			"three rows high as measured in D's font, and D would get the\n"
			"rest of O2's space. That may not be what you want.\n\n");
	glk_window_set_arrangement(o2, winmethod_Below | winmethod_Fixed, 3, NULL);
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_d, "D");
	wait_for_key(win_a);
	
	fprintf(stderr, "To set A to be three rows high as measured in A's font,\n"
			"you would do:\n\n");
	glk_window_set_arrangement(o2, winmethod_Below | winmethod_Fixed, 3, win_a);
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_d, "D");
	wait_for_key(win_a);
	
	fprintf(stderr, "Or you could change O2 to a proportional split:\n\n");
	glk_window_set_arrangement(o2, winmethod_Below | winmethod_Proportional, 30,
							   NULL);
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_d, "D");
	wait_for_key(win_a);
	
	fprintf(stderr, "Or:\n\n");
	glk_window_set_arrangement(o2, winmethod_Above | winmethod_Proportional, 70,
							   NULL);
	center_text(win_a, "A");
	center_text(win_b, "B");
	center_text(win_d, "D");
	wait_for_key(win_a);
	
	glk_window_close(win_d, NULL);
	glk_window_close(win_b, NULL);
	glk_window_clear(win_a);
	glk_set_window(win_a);
	glk_put_string("That's all, folks...");
}
