/*
 * doc.c - Contains the short and long descriptions of all the documentation
 *         sections in the Glk spec, as well as the GtkDoc comments for symbols
 *         defined only in glk.h.
 */

/**
 * SECTION:glk-exiting
 * @short_description: How to terminate a Glk program cleanly
 *
 * A Glk program usually ends when the end of the glk_main() function is 
 * reached. You can also terminate it earlier.
 */ 

/**
 * SECTION:glk-interrupt
 * @short_description: Specifying an interrupt handler for cleaning up critical
 * resources
 *
 * Most platforms have some provision for interrupting a program
 * &mdash; <keycombo action="simul"><keycap
 * function="command">command</keycap><keycap>period</keycap></keycombo> on the
 * Macintosh, <keycombo action="simul"><keycap
 * function="control">control</keycap><keycap>C</keycap></keycombo> in Unix,
 * possibly a window manager item, or other possibilities.
 * This can happen at any time, including while execution is nested inside one
 * of your own functions, or inside a Glk library function.
 *
 * If you need to clean up critical resources, you can specify an interrupt
 * handler function.
 */

/**
 * SECTION:glk-tick
 * @short_description: Yielding time to the operating system
 *
 * Many platforms have some annoying thing that has to be done every so often,
 * or the gnurrs come from the voodvork out and eat your computer.
 * 
 * Well, not really. But you should call glk_tick() every so often, just in
 * case. It may be necessary to yield time to other applications in a
 * cooperative-multitasking OS, or to check for player interrupts in an infinite
 * loop.
 */

/**
 * SECTION:glk-types
 * @short_description: Basic types used in Glk
 *
 * For simplicity, all the arguments used in Glk calls are of a very few types.
 * <variablelist>
 *  <varlistentry>
 *    <term>32-bit unsigned integer</term>
 *    <listitem><para>Unsigned integers are used wherever possible, which is
 *    nearly everywhere. This type is called #glui32.</para></listitem>
 *  </varlistentry>
 *  <varlistentry>
 *    <term>32-bit signed integer</term>
 *    <listitem><para>This type is called #glsi32. Rarely used.</para>
 *    </listitem>
 *  </varlistentry>
 *  <varlistentry>
 *    <term>References to library objects</term>
 *    <listitem><para>These are pointers to opaque C structures; each library
 *    will use different structures, so you can not and should not try to
 *    manipulate their contents. See <link 
 *    linkend="chimara-Opaque-Objects">Opaque Objects</link>.</para></listitem>
 *  </varlistentry>
 *  <varlistentry>
 *    <term>Pointer to one of the above types</term>
 *    <listitem><para>Pointer to a structure which consists entirely of the
 *    above types.</para></listitem>
 *  </varlistentry>
 *  <varlistentry>
 *    <term><type>unsigned char</type></term> 
 *    <listitem><para>This is used only for Latin-1 text characters; see 
 *    <link linkend="chimara-Character-Encoding">Character Encoding</link>.
 *    </para></listitem>
 *  </varlistentry>
 *  <varlistentry>
 *    <term>Pointer to <type>char</type></term> 
 *    <listitem><para>Sometimes this means a null-terminated string; sometimes
 *    an unterminated buffer, with length as a separate #glui32 argument. The
 *    documentation says which.</para></listitem>
 *  </varlistentry>
 *  <varlistentry>
 *    <term>Pointer to <type>void</type></term> 
 *    <listitem><para>When nothing else will do.</para></listitem>
 *  </varlistentry>
 * </variablelist>
 */

/**
 * SECTION:glk-opaque-objects
 * @short_description: Complex objects in Glk
 *
 * Glk keeps track of a few classes of special objects. These are opaque to your
 * program; you always refer to them using pointers to opaque C structures.
 * 
 * Currently, these classes are:
 * <variablelist>
 *  <varlistentry>
 *    <term>Windows</term>
 *    <listitem><para>Screen panels, used to input or output information.
 *    </para></listitem>
 *  </varlistentry>
 *  <varlistentry>
 *    <term>Streams</term>
 *    <listitem><para>Data streams, to which you can input or output text.
 *    </para>
 *    <note><para>There are file streams and window streams, since you can
 *    output data to windows or files.</para></note>
 *    </listitem>
 *  </varlistentry>
 *  <varlistentry>
 *    <term>File references</term>
 *    <listitem><para>Pointers to files in permanent storage.</para>
 *    <note><para>In Unix a file reference is a pathname; on the Mac, an
 *    <type>FSSpec</type>. Actually there's a little more information included,
 *    such as file type and whether it is a text or binary file.</para></note>
 *    </listitem>
 *  </varlistentry>
 *  <varlistentry>
 *    <term>Sound channels</term>
 *    <listitem><para>Audio output channels.</para>
 *    <note><para>Not all Glk libraries support sound.</para></note>
 *    </listitem>
 *  </varlistentry>
 * </variablelist>
 *
 * <note><para>
 * Note that there may be more object classes in future versions of the Glk API.
 * </para></note>
 * 
 * When you create one of these objects, it is always possible that the creation
 * will fail (due to lack of memory, or some other OS error.) When this happens,
 * the allocation function will return %NULL instead of a valid pointer. You
 * should always test for this possibility.
 * 
 * %NULL is never the identifier of any object (window, stream, file reference,
 * or sound channel). The value %NULL is often used to indicate “no object” or
 * “nothing”, but it is not a valid reference.
 * If a Glk function takes an object reference as an argument, it is illegal to
 * pass in %NULL unless the function definition says otherwise.
 * 
 * The `glk.h` file defines types #winid_t, #strid_t, #frefid_t, #schanid_t to
 * store references.
 * These are pointers to struct #glk_window_struct, #glk_stream_struct,
 * #glk_fileref_struct, and #glk_schannel_struct respectively. It is, of course,
 * illegal to pass one kind of pointer to a function which expects another.
 * 
 * <note><para>
 * This is how you deal with opaque objects from a C program. If you are using
 * Glk through a virtual machine, matters will probably be different. Opaque
 * objects may be represented as integers, or as VM objects of some sort.
 * </para></note>
 *
 * # Rocks # {#chimara-Rocks}
 *
 * Every one of these objects (window, stream, file reference, or sound channel)
 * has a “rock” value.
 * This is simply a 32-bit integer value which you provide, for your own
 * purposes, when you create the object.
 *
 * <note><para>The library &mdash; so to speak &mdash; stuffs this value under a
 * rock for safe-keeping, and gives it back to you when you ask for it.
 * </para></note>
 * <note><para>If you don't know what to use the rocks for, provide 0 and forget
 * about it.</para></note>
 *
 * # Iteration Through Opaque Objects # {#chimara-Iterating-Through-Opaque-Objects}
 *
 * For each class of opaque objects, there is an iterate function, which you can
 * use to obtain a list of all existing objects of that class. It takes the form
 * |[<!--language="C"-->
 * CLASSid_t glk_CLASS_iterate(CLASSid_t obj, glui32 *rockptr);
 * ]|
 * ...where `<replaceable>CLASS</replaceable>` represents one of the opaque
 * object classes.
 *
 * <note><para>
 *   So, at the current time, these are the functions glk_window_iterate(),
 *   glk_stream_iterate(), glk_fileref_iterate(), and glk_schannel_iterate().  
 *   There may be more classes in future versions of the spec; they all behave
 *   the same.
 * </para></note>
 *
 * Calling `glk_<replaceable>CLASS</replaceable>_iterate(NULL, r)` returns the
 * first object; calling `glk_<replaceable>CLASS</replaceable>_iterate(obj, r)`
 * returns the next object, until there aren't any more, at which time it
 * returns %NULL.
 *
 * The @rockptr argument is a pointer to a location; whenever  
 * `glk_<replaceable>CLASS</replaceable>_iterate()` returns an object, the
 * object's rock is stored in the location `(*@rockptr)`.
 * If you don't want the rocks to be returned, you may set @rockptr to %NULL.
 *
 * You usually use this as follows:
 * |[<!--language="C"-->
 * obj = glk_CLASS_iterate(NULL, NULL);
 * while (obj) {
 *    // ...do something with obj...
 *    obj = glk_CLASS_iterate(obj, NULL);
 * }
 * ]|
 *
 * If you create or destroy objects inside this loop, obviously, the results are
 * unpredictable. However it is always legal to call 
 * `glk_<replaceable>CLASS</replaceable>_iterate(obj, r)` as long as @obj is a
 * valid object id, or %NULL.
 *
 * The order in which objects are returned is entirely arbitrary. The library
 * may even rearrange the order every time you create or destroy an object of
 * the given class. As long as you do not create or destroy any object, the rule
 * is that `glk_<replaceable>CLASS</replaceable>_iterate(obj, r)` has a fixed
 * result, and iterating through the results as above will list every object
 * exactly once.
 */

/**
 * SECTION:glk-gestalt
 * @short_description: Testing Glk's capabilities
 *
 * The “gestalt” mechanism (cheerfully stolen from the Mac OS) is a system by
 * which the Glk API can be upgraded without making your life impossible.
 * New capabilities (graphics, sound, or so on) can be added without changing
 * the basic specification.
 * The system also allows for “optional” capabilities &mdash; those which not
 * all Glk library implementations will support &mdash; and allows you to check
 * for their presence without trying to infer them from a version number.
 *
 * The basic idea is that you can request information about the capabilities of
 * the API, by calling the gestalt functions.
 */

/**
 * SECTION:glk-character-input
 * @short_description: Waiting for a single keystroke
 *
 * You can request that the player hit a single key. See [Character Input
 * Events][chimara-Character-Input-Events].
 *
 * If you use the basic text API, the character code which is returned can be
 * any value from 0 to 255. The printable character codes have already been
 * described.
 * The remaining codes are typically control codes: <keycombo
 * action="simul"><keycap function="control">control</keycap><keycap>A</keycap></keycombo>
 * to <keycombo action="simul"><keycap
 * function="control">control</keycap><keycap>Z</keycap></keycombo> and a few
 * others.
 *
 * There are also a number of special codes, representing special keyboard
 * keys, which can be returned from a char-input event. These are represented
 * as 32-bit integers, starting with 4294967295 (0xFFFFFFFF) and working down.
 * The special key codes are defined in the `glk.h` file.
 * They include one code for <keycap function="enter">return</keycap> or <keycap
 * function="enter">enter</keycap>, one for <keycap
 * function="delete">delete</keycap> or <keycap
 * function="backspace">backspace</keycap>, twelve function keys, and one code
 * for any key which has no Latin-1 or special code. The full list of key codes
 * is included below.
 * 
 * Various implementations of Glk will vary widely in which characters the
 * player can enter. The most obvious limitation is that some characters are
 * mapped to others. For example, most keyboards return a <keycombo
 * action="simul"><keycap function="control">control</keycap><keycap>I</keycap>
 * </keycombo> code when the <keycap function="tab">tab</keycap> key is
 * pressed. The Glk library, if it can recognize this at all, will generate a
 * %keycode_Tab event (value 0xFFFFFFF7) when this occurs.
 * Therefore, for these keyboards, no keyboard key will generate a <keycombo
 * action="simul"><keycap function="control">control</keycap><keycap>I</keycap>
 * </keycombo> event (value 9.) The Glk library will probably map many of the
 * control codes to the other special keycodes.
 * 
 * <note><para>
 *   On the other hand, the library may be very clever and discriminate between
 *   <keycap>tab</keycap> and <keycombo action="simul"><keycap
 *   function="control">control</keycap><keycap>I</keycap></keycombo>. This is
 *   legal. The idea is, however, that if your program asks the player to
 *   “<computeroutput>press the <keycap function="tab">tab</keycap>
 *   key</computeroutput>”, you should check for a
 *   %keycode_Tab event as opposed to a <keycombo action="simul"><keycap
 *   function="control">control</keycap><keycap>I</keycap></keycombo> event.
 * </para></note>
 * 
 * Some characters may not be enterable simply because they do not exist.
 * 
 * <note><para>
 *   Not all keyboards have a <keycap function="home">home</keycap> or <keycap
 *   function="end">end</keycap> key. A pen-based platform may not recognize
 *   any control characters at all.
 * </para></note>
 * 
 * Some characters may not be enterable because they are reserved for the
 * purposes of the interface.
 * For example, the Mac Glk library reserves the <keycap
 * function="tab">tab</keycap> key for switching between different Glk windows.
 * Therefore, on the Mac, the library will never generate a %keycode_Tab event
 * or a <keycombo action="simul"><keycap
 * function="control">control</keycap><keycap>I</keycap></keycombo> event.
 * 
 * <note><para>
 *   Note that the linefeed or <keycombo action="simul"><keycap  
 *   function="control">control</keycap><keycap>J</keycap></keycombo> 
 *   character, which is the only printable control character, is probably not
 *   typable. This is because, in most libraries, it will be converted to
 *   %keycode_Return.
 *   Again, you should check for %keycode_Return if your program asks the player
 *   to “<computeroutput>press the <keycap function="enter">return</keycap>
 *   key</computeroutput>”.
 * </para></note>
 * 
 * <note><para>
 *   The <keycap function="delete">delete</keycap> and <keycap
 *   function="backspace">backspace</keycap> keys are merged into a single
 *   keycode because they have such an astonishing history of being confused in
 *   the first place... this spec formally waives any desire to define the
 *   difference. Of course, a library is free to distinguish <keycap
 *   function="delete">delete</keycap> and <keycap
 *   function="backspace">backspace</keycap> during line input. This is when it
 *   matters most; conflating the two during character input should not be a
 *   large problem.
 * </para></note>
 *
 * You can test for this by using the %gestalt_CharInput selector.
 * 
 * <note><para>
 *   Glk porters take note: it is not a goal to be able to generate every
 *   single possible key event. If the library says that it can generate a
 *   particular keycode, then game programmers will assume that it is
 *   available, and ask players to use it.
 *   If a %keycode_Home event can only be generated by typing <keycombo
 *   action="seq"><keycap function="escape">escape</keycap><keycombo
 *   action="simul"><keycap
 *   function="control">control</keycap><keycap>A</keycap></keycombo>
 *   </keycombo>, and the player does not know this, the player will be lost
 *   when the game says “<computeroutput>Press the <keycap
 *   function="home">home</keycap> key to see the next 
 *   hint.</computeroutput>” It is better for the library to say that it
 *   cannot generate a %keycode_Home event; that way the game can detect the
 *   situation and ask the user to type <keycap>H</keycap> instead.
 * </para>
 * <para>
 *   Of course, it is better not to rely on obscure keys in any case. The arrow
 *   keys and <keycap function="enter">return</keycap> are nearly certain to be
 *   available; the others are of gradually decreasing reliability, and you
 *   (the game programmer) should not depend on them. You must be certain to
 *   check for the ones you want to use, including the arrow keys and <keycap
 *   function="enter">return</keycap>, and be prepared to use different keys in
 *   your interface if %gestalt_CharInput says they are not available.
 * </para></note>
 */

/**
 * SECTION:glk-case
 * @short_description: Changing the case of strings
 *
 * Glk has functions to manipulate the case of both Latin-1 and Unicode strings.
 * One Latin-1 lowercase character corresponds to one uppercase character, and
 * vice versa, so the Latin-1 functions act on single characters. The Unicode
 * functions act on whole strings, since the length of the string may change.
 */

/**
 * SECTION:glk-normalize
 * @short_description: Combining characters
 *
 * Comparing Unicode strings is difficult, because there can be several ways to
 * represent a piece of text as a Unicode string. For example, the one-character
 * string “&egrave;” (an accented “e”) will be displayed the same as the
 * two-character string containing “e” followed by Unicode character 0x0300
 * (COMBINING GRAVE ACCENT).
 * These strings should be considered equal.
 *
 * Therefore, a Glk program that accepts line input should convert its text to a
 * normalized form before parsing it. These functions offer those conversions.
 * The algorithms are defined by the Unicode spec (chapter 3.7) and [Unicode
 * Standard Annex &num;15](http://www.unicode.org/reports/tr15/).
 */

/**
 * SECTION:glk-window-opening
 * @short_description: Creating new windows and closing them
 *
 * You can open a new window using glk_window_open() and close it again using
 * glk_window_close().
 */

/**
 * SECTION:glk-window-constraints
 * @short_description: Manipulating the size of a window
 *
 * There are library functions to change and to measure the size of a window.
 */

/**
 * SECTION:glk-window-types
 * @short_description: Blank, pair, text grid, text buffer, and graphics windows
 * 
 * A technical description of all the window types, and exactly how they behave.
 */

/**
 * SECTION:glk-echo-streams
 * @short_description: Creating a copy of a window's output
 *
 * Every window has an associated window stream; you print to the window by
 * printing to this stream. However, it is possible to attach a second stream to
 * a window. Any text printed to the window is also echoed to this second
 * stream, which is called the window's “echo stream.”
 *
 * Effectively, any call to glk_put_char() (or the other output commands) which
 * is directed to the window's window stream, is replicated to the window's echo
 * stream. This also goes for the style commands such as glk_set_style().
 * 
 * Note that the echoing is one-way. You can still print text directly to the
 * echo stream, and it will go wherever the stream is bound, but it does not
 * back up and appear in the window. 
 *
 * An echo stream can be of any type, even another window's window stream.
 * 
 * <note><para>
 *   This would be somewhat silly, since it would mean that any text printed to
 *   the window would be duplicated in another window. More commonly, you would
 *   set a window's echo stream to be a file stream, in order to create a
 *   transcript file from that window.
 * </para></note>
 *
 * A window can only have one echo stream. But a single stream can be the echo
 * stream of any number of windows, sequentially or simultaneously.
 *
 * If a window is closed, its echo stream remains open; it is not automatically
 * closed. 
 *
 * <note><para>
 *   Do not confuse the window's window stream with its echo stream. The window
 *   stream is “owned” by the window, and dies with it.
 *   The echo stream is merely temporarily associated with the window.
 * </para></note>
 * 
 * If a stream is closed, and it is the echo stream of one or more windows,
 * those windows are reset to not echo anymore. (So then calling
 * glk_window_get_echo_stream() on them will return %NULL.) 
 */

/**
 * SECTION:glk-window-other
 * @short_description: Miscellaneous functions for windows
 *
 * This section contains functions for windows that don't fit anywhere else.
 */

/**
 * SECTION:glk-events
 * @short_description: Waiting for events
 *
 * As described in [Your Program's Main
 * Function][chimara-Your-Programs-Main-Function], all player input is handed to
 * your program by the glk_select() call, in the form of events.
 * You should write at least one event loop to retrieve these events.
 */

/**
 * SECTION:glk-character-input-events
 * @short_description: Events representing a single keystroke
 *
 * You can request character input from text buffer, text grid, and graphics
 * windows.
 * See %evtype_CharInput.
 * There are separate functions for requesting the availability of particular
 * Latin-1 and Unicode characters; see %gestalt_Unicode.
 * To test whether graphics windows support character input, use the
 * %gestalt_GraphicsCharInput selector.
 */

/**
 * SECTION:glk-line-input-events
 * @short_description: Events representing a line of user input
 *
 * You can request line input from text buffer and text grid windows. See
 * %evtype_LineInput.
 * There are separate functions for requesting the availability of particular
 * Latin-1 and Unicode characters; see %gestalt_Unicode.
 */

/**
 * SECTION:glk-mouse-events
 * @short_description: Events representing a mouse click
 *
 * On some platforms, Glk can recognize when the mouse (or other pointer) is 
 * used to select a spot in a window. You can request mouse input only in text 
 * grid windows and graphics windows.
 * 
 * A window can have mouse input and character/line input pending at the same
 * time.
 * 
 * If the player clicks in a window which has a mouse input event pending,
 * glk_select() will return an event whose type is %evtype_MouseInput. Again,
 * once this happens, the request is complete, and you must request another if
 * you want further mouse input.
 * 
 * In the event structure, @win tells what window the event came from.
 * 
 * In a text grid window, the @val1 and @val2 fields are the x and y coordinates
 * of the character that was clicked on. 
 * <note><para>So @val1 is the column, and @val2 is the row.</para></note>
 * The top leftmost character is considered to be (0,0).
 * 
 * In a graphics window, they are the x and y coordinates of the pixel that was
 * clicked on. Again, the top left corner of the window is (0,0).
 * 
 * <note><para>
 *   Most mouse-based idioms define standard functions for mouse hits in text
 *   windows &mdash; typically selecting or copying text. It is up to the
 *   library to separate this from Glk mouse input. The library may choose to
 *   select text when it is clicked normally, and cause Glk mouse events when
 *   text is control-clicked. Or the other way around. Or it may be the
 *   difference between clicking and double-clicking. Or the library may
 *   reserve a particular mouse button, on a multi-button mouse. It may even
 *   specify a keyboard key to be the "mouse button", referring to wherever the
 *   mouse cursor is when the key is hit. Or some even more esoteric positioning
 *   system. You need only know that the user can do it, or not.
 * </para></note> 
 * <note><para>
 *   However, since different platforms will handle this issue differently, you
 *   should be careful how you instruct the player in your program. Do not tell
 *   the player to “double-click”, “right-click”, or “control-click” in a
 *   window.
 *   The preferred term is “to touch the window”, or a spot in the window.
 * </para></note>
 * <note><para>
 *   Goofy, but preferred.
 * </para></note>
 */

/**
 * SECTION:glk-timer-events
 * @short_description: Events sent at fixed intervals
 *
 * You can request that an event be sent at fixed intervals, regardless of what
 * the player does. Unlike input events, timer events can be tested for with
 * glk_select_poll() as well as glk_select().
 *
 * It is possible that the library does not support timer events. You can check
 * this with the %gestalt_Timer selector.
 */
 
/**
 * SECTION:glk-streams
 * @short_description: Input and output abstractions
 *
 * All character output in Glk is done through streams. Every window has an
 * output stream associated with it. You can also write to files on disk; every
 * open file is represented by an output stream as well.
 *
 * There are also input streams; these are used for reading from files on disk.
 * It is possible for a stream to be both an input and an output stream. 
 *
 * <note><para>
 *   Player input is done through line and character input events, not streams.
 *   This is a small inelegance in theory. In practice, player input is slow and
 *   things can interrupt it, whereas file input is immediate. If a network
 *   extension to Glk were proposed, it would probably use events and not
 *   streams, since network communication is not immediate.
 * </para></note>
 *
 * It is also possible to create a stream that reads or writes to a buffer in
 * memory.
 * 
 * Finally, there may be platform-specific types of streams, which are created
 * before your program starts running. 
 *
 * <note><para>
 *   For example, a program running under Unix may have access to standard input
 *   as a stream, even though there is no Glk call to explicitly open standard
 *   input. On the Mac, data in a Mac resource may be available through a
 *   resource-reading stream.
 * </para></note>
 *
 * You do not need to worry about the origin of such streams; just read or write
 * them as usual. For information about how platform-specific streams come to
 * be, see [Startup Options][chimara-Startup-Options].
 *
 * A stream is opened with a particular file mode, see the `filemode_` constants
 * below.
 *
 * <note><para>
 *   In the stdio library, using fopen(), %filemode_Write would be mode
 *   <code>"w"</code>; %filemode_Read would be mode <code>"r"</code>;
 *   %filemode_ReadWrite would be mode <code>"r+"</code>. Confusingly,
 *   %filemode_WriteAppend cannot be mode <code>"a"</code>, because the stdio
 *   spec says that when you open a file with mode <code>"a"</code>, then
 *   fseek() doesn't work. So we have to use mode <code>"r+"</code> for
 *   appending. Then we run into the <emphasis>other</emphasis> stdio problem,
 *   which is that <code>"r+"</code> never creates a new file. So
 *   %filemode_WriteAppend has to <emphasis>first</emphasis> open the file with
 *   <code>"a"</code>, close it, reopen with <code>"r+"</code>, and then
 *   fseek() to the end of the file. For %filemode_ReadWrite, the process is
 *   the same, except without the fseek() &mdash; we begin at the beginning of
 *   the file.
 * </para></note>
 * <note><para>
 *   We must also obey an obscure geas of ANSI C <code>"r+"</code> files: you
 *   can't switch from reading to writing without doing an fseek() in between.
 *   Switching from writing to reading has the same restriction, except that an
 *   fflush() also works.
 * </para></note>
 *
 * For information on opening streams, see the discussion of each specific type
 * of stream in [The Types of Streams][chimara-The-Types-of-Streams].
 * Remember that it is always possible that opening a stream will fail, in which
 * case the creation function will return %NULL.
 *
 * Each stream remembers two character counts, the number of characters printed
 * to and read from that stream. The write-count is exactly one per 
 * glk_put_char() call; it is figured before any platform-dependent character
 * cookery. 
 *
 * <note><para>
 *   For example, if a newline character is converted to 
 *   linefeed-plus-carriage-return, the stream's count still only goes up by
 *   one; similarly if an accented character is displayed as two characters.
 * </para></note>
 * 
 * The read-count is exactly one per glk_get_char_stream() call, as long as the
 * call returns an actual character (as opposed to an end-of-file token.) 
 *
 * Glk has a notion of the “current (output) stream”.
 * If you print text without specifying a stream, it goes to the current output
 * stream.
 * The current output stream may be %NULL, meaning that there isn't one.
 * It is illegal to print text to stream %NULL, or to print to the current
 * stream when there isn't one.
 *
 * If the stream which is the current stream is closed, the current stream
 * becomes %NULL. 
 */
 
/**
 * SECTION:glk-print
 * @short_description: Printing to streams
 *
 * You can print Latin-1 and Unicode characters, null-terminated strings, or
 * buffers to any stream. The characters will be converted into the appropriate
 * format for that stream.
 */
 
/**
 * SECTION:glk-read
 * @short_description: Reading from streams
 *
 * You can read Latin-1 or Unicode characters, buffers, or whole lines from any
 * stream. The characters will be converted into the form in which you request
 * them.
 */
 
/**
 * SECTION:glk-closing-streams
 * @short_description: Closing streams and retrieving their character counts
 *
 * When you close a Glk stream, you have the opportunity to examine the
 * character counts &mdash; the number of characters written to or read from the
 * stream.
 */

/**
 * SECTION:glk-stream-positions
 * @short_description: Moving the read/write mark
 *
 * You can set the position of the read/write mark in a stream.
 *
 * <note><para>
 *   Which makes one wonder why they're called “streams” in the first place.
 *   Oh well.
 * </para></note>
 */

/**
 * SECTION:glk-styles
 * @short_description: Changing the appearance of printed text
 *
 * You can send style-changing commands to an output stream. After a style
 * change, new text which is printed to that stream will be given the new style,
 * whatever that means for the stream in question. For a window stream, the text
 * will appear in that style. For a memory stream, style changes have no effect.
 * For a file stream, if the machine supports styled text files, the styles may
 * be written to the file; more likely the style changes will have no effect.
 * 
 * Styles are exclusive. A character is shown with exactly one style, not a 
 * subset of the possible styles.
 *
 * <note><para>
 *  Note that every stream and window has its own idea of the “current style.”
 *  Sending a style command to one window or stream does not affect any others.
 * </para></note>
 * <note><para>
 *  Except for a window's echo stream; see <link 
 *  linkend="chimara-Echo-Streams">Echo Streams</link>.
 * </para></note>
 * 
 * The styles are intended to distinguish meaning and use, not formatting. There
 * is no standard definition of what each style will look like. That is left up
 * to the Glk library, which will choose an appearance appropriate for the
 * platform's interface and the player's preferences.
 * 
 * There are currently eleven styles defined. More may be defined in the future.
 * 
 * Styles may be distinguished on screen by font, size, color, indentation,
 * justification, and other attributes. Note that some attributes (notably
 * justification and indentation) apply to entire paragraphs. If possible and
 * relevant, you should apply a style to an entire paragraph &mdash; call 
 * glk_set_style() immediately after printing the newline at the beginning of
 * the text, and do the same at the end.
 * 
 * <note><para>
 *  For example, %style_Header may well be centered text. If you print 
 *  “Welcome to Victim (a short interactive mystery)”, and only the word
 *  “Victim” is in the %style_Header, the center-justification attribute will be
 *  lost.
 *  Similarly, a block quote is usually indented on both sides, but indentation
 *  is only meaningful when applied to an entire line or paragraph, so block
 *  quotes should take up an entire paragraph.
 *  Contrariwise, %style_Emphasized need not be used on an entire paragraph.
 *  It is often used for single emphasized words in normal text, so you can
 *  expect that it will appear properly that way; it will be displayed in
 *  italics or underlining, not center-justified or indented.
 * </para></note> 
 * 
 * <note><para>
 *  Yes, this is all a matter of mutual agreement between game authors and game
 *  players. It's not fixed by this specification. That's natural language for
 *  you.
 * </para></note>
 */

/**
 * SECTION:glk-stylehints
 * @short_description: Setting style hints
 *
 * There are no guarantees of how styles will look, but you can make 
 * suggestions.
 *
 * Initially, no hints are set for any window type or style. Note that having no
 * hint set is not the same as setting a hint with value 0.
 * 
 * These functions do <emphasis>not</emphasis> affect 
 * <emphasis>existing</emphasis> windows. They affect the windows which you
 * create subsequently. If you want to set hints for all your game windows, call
 * glk_stylehint_set() before you start creating windows. If you want different
 * hints for different windows, change the hints before creating each window.
 * 
 * <note><para>
 *  This policy makes life easier for the interpreter. It knows everything about
 *  a particular window's appearance when the window is created, and it doesn't
 *  have to change it while the window exists.
 * </para></note>
 * 
 * Hints are hints. The interpreter may ignore them, or give the player a choice
 * about whether to accept them. Also, it is never necessary to set hints. You
 * don't have to suggest that %style_Preformatted be fixed-width, or 
 * %style_Emphasized be boldface or italic; they will have appropriate defaults.
 * Hints are for situations when you want to <emphasis>change</emphasis> the 
 * appearance of a style from what it would ordinarily be. The most common case
 * when this is appropriate is for the styles %style_User1 and %style_User2.
 * 
 * There are currently ten style hints defined. More may be defined in the 
 * future. 
 * 
 * Again, when passing a style hint to a Glk function, any value is actually 
 * legal. If the interpreter does not recognize the stylehint value, it will 
 * ignore it. 
 * <note><para>
 *  This policy allows for the future definition of style hints without breaking
 *  old Glk libraries.
 * </para></note> 
 */

/**
 * SECTION:glk-style-measure
 * @short_description: Finding out how the library displays your style hints
 *
 * You can suggest the appearance of a window's style before the window is
 * created; after the window is created, you can test the style's actual
 * appearance. These functions do not test the style hints; they test the
 * attribute of the style as it appears to the player.
 *
 * Note that although you cannot change the appearance of a window's styles
 * after the window is created, the library can. A platform may support dynamic
 * preferences, which allow the player to change text formatting while your
 * program is running.
 * <note><para>
 *   Changes that affect window size (such as font size changes) will be
 *   signalled by an %evtype_Arrange event. However, more subtle changes (such
 *   as text color differences) are not signalled. If you test the appearance of
 *   styles at the beginning of your program, you must keep in mind the
 *   possibility that the player will change them later.
 * </para></note>
 */

/**
 * SECTION:glk-stream-types
 * @short_description: Window, memory, file, and resource streams
 *
 * # Window Streams # {#chimara-Window-Streams}
 *
 * Every window has an output stream associated with it. This is created
 * automatically, with %filemode_Write, when you open the window. You get it
 * with glk_window_get_stream(). Window streams always have rock value 0.
 * 
 * A window stream cannot be closed with glk_stream_close(). It is closed
 * automatically when you close its window with glk_window_close().
 * 
 * Only printable characters (including newline) may be printed to a window
 * stream.
 * See [Character Encoding](chimara-Character-Encoding).
 *
 * # Memory Streams # {#chimara-Memory-Streams}
 *
 * You can open a stream which reads from or writes to a space in memory. See
 * glk_stream_open_memory() and glk_stream_open_memory_uni(). When opening a
 * memory stream, you specify a buffer to which the stream's output will be
 * written, and its length @buflen.
 *
 * When outputting, if more than @buflen characters are written to the stream,
 * all of them beyond the buffer length will be thrown away, so as not to
 * overwrite the buffer. (The character count of the stream will still be
 * maintained correctly. That is, it will count the number of characters written
 * into the stream, not the number that fit into the buffer.)
 *
 * If the buffer is %NULL, or for that matter if @buflen is zero, then 
 * <emphasis>everything</emphasis> written to the stream is thrown away. This
 * may be useful if you are interested in the character count.
 *
 * When inputting, if more than @buflen characters are read from the stream, the
 * stream will start returning -1 (signalling end-of-file.) If the buffer is 
 * %NULL, the stream will always return end-of-file.
 *
 * The data is written to the buffer exactly as it was passed to the printing
 * functions (glk_put_char(), etc.); input functions will read the data exactly
 * as it exists in memory. No platform-dependent cookery will be done on it.
 *
 * <note><para>
 *   You can write a disk file in text mode, but a memory stream is effectively
 *   always in binary mode.
 * </para></note>
 * 
 * Whether reading or writing, the contents of the buffer are undefined until
 * the stream is closed. The library may store the data there as it is written,
 * or deposit it all in a lump when the stream is closed. It is illegal to
 * change the contents of the buffer while the stream is open.
 *
 * # File Streams # {#chimara-File-Streams}
 *
 * You can open a stream which reads from or writes to a disk file. See 
 * glk_stream_open_file() and glk_stream_open_file_uni().
 *
 * The file may be written in text or binary mode; this is determined by the
 * file reference you open the stream with. Similarly, platform-dependent
 * attributes such as file type are determined by the file reference.
 * See [File References](chimara-File-References).
 *
 * # Resource Streams # {#chimara-Resource-Streams}
 *
 * You can open a stream which reads from (but not writes to) a resource file.
 *
 * <note><para>
 *   Typically this is embedded in a Blorb file, as Blorb is the official
 *   resource-storage format of Glk. A Blorb file can contain images and sounds,
 *   but it can also contain raw data files, which are accessed by
 *   glk_stream_open_resource() and glk_stream_open_resource_uni(). A data file
 *   is identified by number, not by a filename. The Blorb usage field will be
 *   <code>'Data'</code>. The chunk type will be %giblorb_ID_TEXT for text
 *   resources, %giblorb_ID_BINA or <code>'FORM'</code> for binary resources.
 * </para></note>
 *
 * <note><para>
 *   For a `'FORM'` Blorb chunk, the stream should start reading at the
 *   beginning of the chunk header &mdash; that is, it should read the `'FORM'`
 *   and length words before the chunk content.
 *   For %giblorb_ID_TEXT and %giblorb_ID_BINA chunks, the stream should skip
 *   the header and begin with the chunk content.
 *   This distinction is important when embedding AIFF sounds or Quetzal saved
 *   games, for example.
 * </para></note>
 *
 * <note><para>
 *   Note that this `FORM` distinction was added to the Glk 0.7.4 spec in July
 *   2012, several months after the spec went out.
 *   This is bad form, no pun intended, but I don't think it'll cause headaches.
 *   No games use the resource stream feature yet, as far as I know.
 *   A Glk library written in the interregnum of early 2012 will fail to
 *   recognize `FORM` chunks, and if a game tries to use one,
 *   glk_stream_open_resource() will return %NULL.
 * </para></note>
 *
 * <note><para>
 *   If the running program is not associated with a Blorb file, the library may
 *   look for data files as actual files instead. These would be named
 *   <filename>DATA1</filename>, <filename>DATA2</filename>, etc, with a suffix
 *   distinguishing text and binary files. See “Other Resource Arrangements”
 *   in the Blorb spec: <ulink url="http://eblong.com/zarf/blorb/"></ulink>.
 *   The stream should always begin at the beginning of the file, in this case;
 *   there is no `BINA`/`FORM` distinction to worry about.
 * </para></note>
 */
 
/**
 * SECTION:glk-stream-other
 * @short_description: Miscellaneous functions for streams
 *
 * This section includes functions for streams that don't fit anywhere else.
 */

/**
 * SECTION:glk-fileref
 * @short_description: A platform-independent way to refer to disk files
 *
 * You deal with disk files using file references. Each fileref is an opaque C
 * structure pointer; see [Opaque Objects][chimara-Opaque-Objects].
 *
 * A file reference contains platform-specific information about the name and
 * location of the file, and possibly its type, if the platform has a notion of
 * file type. It also includes a flag indication whether the file is a text file
 * or binary file. 
 *
 * <note><para>
 *   Note that this is different from the standard C I/O library, in which you
 *   specify text or binary mode when the file is opened.
 * </para></note>
 * 
 * A fileref does not have to refer to a file which actually exists. You can
 * create a fileref for a nonexistent file, and then open it in write mode to
 * create a new file.
 * 
 * You always provide a usage argument when you create a fileref. The usage
 * indicates the file type and the mode (text or binary.) It must be the
 * logical-or of a file-type constant and a mode constant. These values are used
 * when you create a new file, and also to filter file lists when the player is
 * selecting a file to load.
 * 
 * In general, you should use text mode if the player expects to read the file
 * with a platform-native text editor; you should use binary mode if the file is
 * to be read back by your program, or if the data must be stored exactly. Text
 * mode is appropriate for %fileusage_Transcript; binary mode is appropriate for
 * %fileusage_SavedGame and probably for %fileusage_InputRecord. %fileusage_Data
 * files may be text or binary, depending on what you use them for.
 *
 * When a fileref is created via a user prompt (glk_fileref_create_by_prompt()),
 * it may include extra file type information.
 *
 * <note><para>
 *   For example, a prompt to write a transcript file might include a choice of
 *   text encodings, or even alternate formats such as RTF or HTML.
 * </para></note>
 *
 * This player-selected information will override the default encoding rules
 * noted above.
 * When a fileref is created non-interactively (glk_fileref_create_by_name(),
 * glk_fileref_create_temp()) the default rules always apply.
 *
 * <note><para>
 *   See also the comments about encoding, [File Streams][chimara-File-Streams].
 * </para></note>
 */
 
/**
 * SECTION:glk-fileref-types
 * @short_description: Four different ways to create a file reference
 *
 * There are four different functions for creating a fileref, depending on how
 * you wish to specify it. Remember that it is always possible that a fileref
 * creation will fail and return %NULL.
 */
 
/**
 * SECTION:glk-fileref-other
 * @short_description: Miscellaneous functions for file references
 *
 * This section includes functions for file references that don't fit anywhere
 * else.
 */

/**
 * SECTION:glk-image-resources
 * @short_description: Graphics in Glk
 *
 * In accordance with this modern age, Glk provides for a modicum of graphical
 * flair. It does not attempt to be a complete graphical toolkit. Those already
 * exist. Glk strikes the usual uncomfortable balance between power, 
 * portability, and ease of implementation: commands for arranging pre-supplied
 * images on the screen and intermixed with text.
 * 
 * Graphics is an optional capability in Glk; not all libraries support 
 * graphics. This should not be a surprise.
 * 
 * Most of the graphics commands in Glk deal with image resources. Your program
 * does not have to worry about how images are stored. Everything is a resource,
 * and a resource is referred to by an integer identifier. You may, for example,
 * call a function to display image number 17. The format, loading, and 
 * displaying of that image is entirely up to the Glk library for the platform
 * in question.
 * 
 * Of course, it is also desirable to have a platform-independent way to store
 * sounds and images. Blorb is the official resource-storage format of Glk. A
 * Glk library does not have to understand Blorb, but it is more likely to
 * understand Blorb than any other format.
 *
 * <note><para>
 *   Glk does not specify the exact format of images, but Blorb does. Images in 
 *   a Blorb archive must be PNG or JPEG files. More formats may be added if 
 *   real-world experience shows it to be desirable. However, that is in the 
 *   domain of the Blorb specification. The Glk spec, and Glk programming, will
 *   not change.
 * </para></note>
 * 
 * At present, images can only be drawn in graphics windows and text buffer 
 * windows. In fact, a library may not implement both of these possibilities.
 * You should test each with the %gestalt_DrawImage selector if you plan to use
 * it.
 * See [Testing for Graphics
 * Capabilities][chimara-Testing-for-Graphics-Capabilities].
 */

/**
 * SECTION:glk-graphics-windows
 * @short_description: Drawing graphics in graphics windows
 *
 * A graphics window is a rectangular canvas of pixels, upon which you can draw
 * images. The contents are entirely under your control. You can draw as many
 * images as you like, at any positions &mdash; overlapping if you like. If the
 * window is resized, you are responsible for redrawing everything.
 * See [Graphics Windows][wintype-Graphics].
 *
 * <note><para>
 *   Note that graphics windows do not support a full set of object-drawing 
 *   commands, nor can you draw text in them. That may be available in a future 
 *   Glk extension. For now, it seems reasonable to limit the task to a single 
 *   primitive, the drawing of a raster image. And then there's the ability to
 *   fill a rectangle with a solid color &mdash; a small extension, and 
 *   hopefully no additional work for the library, since it can already clear 
 *   with arbitrary background colors. In fact, if glk_window_fill_rect() did 
 *   not exist, an author could invent it &mdash; by briefly setting the
 *   background color, erasing a rectangle, and restoring.
 * </para></note>
 * 
 * If you call glk_image_draw() or glk_image_draw_scaled() in a graphics window,
 * @val1 and @val2 are interpreted as X and Y coordinates. The image will be 
 * drawn with its upper left corner at this position.
 * 
 * It is legitimate for part of the image to fall outside the window; the excess
 * is not drawn. Note that these are signed arguments, so you can draw an image
 * which falls outside the left or top edge of the window, as well as the right
 * or bottom.
 * 
 * There are a few other commands which apply to graphics windows.
 */

/**
 * SECTION:glk-graphics-text
 * @short_description: Drawing graphics inside or beside text
 *
 * A text buffer is a linear text stream. You can draw images in-line with this
 * text. If you are familiar with HTML, you already understand this model. You
 * draw images with flags indicating alignment. The library takes care of
 * scrolling, resizing, and reformatting text buffer windows.
 *
 * If you call glk_image_draw() or glk_image_draw_scaled() in a text buffer
 * window, @val1 gives the image alignment. The @val2 argument is currently
 * unused, and should always be zero.
 *
 * The two “margin” alignments require some care.
 * To allow proper positioning, images using %imagealign_MarginLeft and
 * %imagealign_MarginRight must be placed at the beginning of a line.
 * That is, you may only call glk_image_draw() (with these two alignments) in a
 * window, if you have just printed a newline to the window's stream, or if the
 * window is entirely empty.
 * If you margin-align an image in a line where text has already appeared, no 
 * image will appear at all.
 *
 * Inline-aligned images count as “text” for the purpose of this rule.
 *
 * You may have images in both margins at the same time.
 * 
 * It is also legal to have more than one image in the same margin (left or 
 * right.) However, this is not recommended. It is difficult to predict how text
 * will wrap in that situation, and libraries may err on the side of 
 * conservatism. 
 */

/**
 * SECTION:glk-graphics-testing
 * @short_description: Checking whether the library supports graphics
 *
 * Before calling Glk graphics functions, you should use the gestalt selector
 * %gestalt_Graphics. To test for additional capabilities, you can also use the
 * %gestalt_DrawImage and %gestalt_GraphicsTransparency selectors.
 */

/**
 * SECTION:glk-sound-channels
 * @short_description: Creating new sound channels and closing them
 *
 * Sounds in Glk are played through sound channels. Sound channels are another
 * type of opaque object, like windows, streams, and file references.
 */

/**
 * SECTION:glk-playing-sounds
 * @short_description: Producing noise
 *
 * These functions play the actual sounds through the sound channels.
 */

/**
 * SECTION:glk-sound-other
 * @short_description: Miscellaneous functions for sound channels
 *
 * This section includes functions for sound channels that don't fit anywhere
 * else.
 */

/**
 * SECTION:glk-sound-testing
 * @short_description: Checking whether the library supports sound
 *
 * Before calling Glk sound functions, you should use the %gestalt_Sound2
 * selector.
 *
 * Earlier versions of the Glk spec defined separate selectors for various
 * optional capabilities. This has proven to be an unnecessarily confusing
 * strategy, and is no longer used. The %gestalt_Sound, %gestalt_SoundMusic,
 * %gestalt_SoundVolume, and %gestalt_SoundNotify selectors still exist, but you
 * should not need to test them; the %gestalt_Sound2 selector covers all of
 * them.
 */

/**
 * SECTION:glk-creating-hyperlinks
 * @short_description: Printing text as a hyperlink
 *
 * Some games may wish to mark up text in their windows with hyperlinks, which
 * can be selected by the player &mdash; most likely by mouse click. Glk allows
 * this in a manner similar to the way text styles are set.
 *
 * Hyperlinks are an optional capability in Glk.
 */

/**
 * SECTION:glk-accepting-hyperlinks
 * @short_description: Generating and catching hyperlink navigation events
 *
 * When you request a hyperlink event in a window, you will receive a hyperlink
 * event when the player clicks on a hyperlink.
 */

/**
 * SECTION:glk-hyperlinks-testing
 * @short_description: Checking whether the library supports hyperlinks
 *
 * Before calling Glk hyperlink functions, you should use the gestalt selectors
 * %gestalt_Hyperlinks and %gestalt_HyperlinkInput.
 */

/**
 * SECTION:glk-clock
 * @short_description: Getting the current time from the system clock
 *
 * You can get the current time, either as a Unix timestamp (seconds since 1970)
 * or as a broken-out structure of time elements (year, month, day, hour,
 * minute, second).
 *
 * The system clock is not guaranteed to line up with timer events (see [Timer
 * Events][chimara-Timer-Events]).
 * Timer events may be delivered late according to the system clock.
 */

/**
 * SECTION:glk-clock-conversions
 * @short_description: Converting from timestamps to date structures and back
 *
 * This section describes functions for converting timestamps to more
 * human-readable date structures and back.
 */

/**
 * SECTION:glk-clock-testing
 * @short_description: Checking whether the library supports the clock functions
 *
 * Before calling Glk date and time functions, you should use the
 * %gestalt_DateTime selector.
 */
 
/**
 * SECTION:dispatch-interrogating
 * @short_description: Finding out what functions the Glk library exports
 *
 * These are the ancilliary functions that let you enumerate.
 */
 
/**
 * SECTION:dispatch-dispatching
 * @short_description: Dispatching the call to the Glk library
 *
 * The function gidispatch_call() invokes a function from the Glk library.
 */
 
/**
 * SECTION:dispatch-prototypes
 * @short_description: Querying Glk function prototypes
 *
 * There are many possible ways to set up a #gluniversal_t array, and it's
 * illegal to call gidispatch_call() with an array which doesn't match the
 * function. Furthermore, some references are passed in, some passed out, and
 * some both. How do you know how to handle the argument list?
 * 
 * One possibility is to recognize each function selector, and set up the
 * arguments appropriately. However, this entails writing special code for each
 * Glk function; which is exactly what we don't want to do.
 * 
 * Instead, you can call gidispatch_prototype(). 
 */

/**
 * SECTION:dispatch-game-id
 * @short_description: Querying a game ID string
 * @stability: Unstable
 *
 * These functions are not part of Glk dispatching per se; they allow the game
 * to provide an identifier string for the Glk library to use.
 * The functions themselves are in <filename>gi_dispa.c</filename>.
 */

/**
 * SECTION:dispatch-library-functions
 * @short_description: Platform-dependent dispatch layer functions
 *
 * Ideally, the three layers &mdash; program, dispatch layer, Glk library
 * &mdash; would be completely modular; each would refer only to the layers
 * beneath it. Sadly, there are a few places where the library must notify the
 * program that something has happened. Worse, these situations are only
 * relevant to programs which use the dispatch layer, and then only some of
 * those.
 * 
 * Since C is uncomfortable with the concept of calling functions which may not
 * exist, Glk handles this with call-back function pointers. The program can
 * pass callbacks in to the library; if it does, the library will call them, and
 * if not, the library doesn't try.
 * 
 * These callbacks are optional, in the sense that the program may or may not
 * set them. However, any library which wants to interoperate with the dispatch
 * layer must <emphasis>allow</emphasis> the program to set them; it is the
 * program's choice.
 * The library does this by implementing `set_registry` functions &mdash; the
 * functions to which the program passes its callbacks.
 *
 * <note><para>
 *   Even though these callbacks and the functions to set them are declared in
 *   <filename class="headerfile">gi_dispa.h</filename>, they are not defined in
 *   <filename>gi_dispa.c</filename>. The dispatch layer merely coordinates
 *   them. The program defines the callback functions; the library calls them.
 * </para></note>
 */

/** 
 * SECTION:blorb-program
 * @short_description: How to use the Blorb layer in your program
 *
 * If you wish your program to load its resources from a Blorb file, you need to
 * find and open that file in your startup code.
 * (See [Startup Options][chimara-Startup-Options].)
 * Each platform will have appropriate functions available for finding startup
 * data.
 * Be sure to open the file in binary mode, not text mode.
 * Once you have opened the file as a Glk stream, pass it to
 * giblorb_set_resource_map().
 *
 * If you do not call giblorb_set_resource_map() in your startup code, or if it
 * fails, the library is left to its own devices for finding resources. Some
 * libraries may try to load resources from individual files &mdash; 
 * `PIC1`, `PIC2`, `PIC3`, and so on.
 * (See the Blorb specification for more on this approach.)
 * Other libraries will not have any other loading mechanism at all; no
 * resources will be available.
 */

/**
 * SECTION:blorb-layer
 * @short_description: The platform-independent functions in the Blorb layer
 *
 * These are the functions which are implemented in `gi_blorb.c`.
 * They will be compiled into the library, but they are the same on every
 * platform.
 * In general, only the library needs to call these functions.
 * The Glk program should allow the library to do all the resource handling.
 */

/**
 * SECTION:blorb-errors
 * @short_description: Error codes returned by the Blorb layer functions
 *
 * All Blorb layer functions, including giblorb_set_resource_map(), return the
 * following error codes.
 */

/**
 * SECTION:glkext-startup
 * @short_description: Parsing startup options
 *
 * This section describes an extension to Glk for parsing command-line startup
 * options. It was written by Andrew Plotkin for the Glk libraries CheapGlk and
 * GlkTerm. 
 *
 * When you compile a Glk program, you may define a function called 
 * `glkunix_startup_code()`, and an array `glkunix_arguments[]`.
 * These set up various Unix-specific options used by the Glk library.
 * There is a sample “`glkstart.c`” file included in this package; you should
 * modify it to your needs.
 * |[<!--language="C"-->
 * extern glkunix_argumentlist_t glkunix_arguments[];
 * ]|
 * The `glkunix_arguments[]` array is a list of command-line arguments that your
 * program can accept.
 * The library will sort these out of the command line and pass them on to your
 * code.
 */

/**
 * SECTION:glkext-unix
 * @short_description: Unix-specific functions
 *
 * This section describes an extension to Glk for various Unix functions. It was
 * written by Andrew Plotkin for the Glk libraries CheapGlk and GlkTerm.
 *
 * You can put other startup code in glkunix_startup_code(). This should
 * generally be limited to finding and opening data files. There are a few Unix
 * Glk library functions which are convenient for this purpose.
 */

/**
 * SECTION:glkext-garglk
 * @short_description: Gargoyle extensions to Glk
 *
 * This section describes various extensions to Glk that were written for the
 * popular interpreter [Gargoyle](http://www.ccxvii.net/gargoyle/) by Tor
 * Andersson (now maintained by Ben Cressey).
 *
 * These functions mostly serve to close the gap between Glk's input/output
 * capabilities and what some interpreters expect. For example, 
 * garglk_set_zcolors() displays the colors defined in the Z-machine standard,
 * and garglk_set_story_name() can be used to give the host program a hint
 * about what to display in the title bar of its window.
 */ 
 
/*---------------- TYPES AND CONSTANTS FROM GLK.H ----------------------------*/

/**
 * glui32:
 *
 * A 32-bit unsigned integer type, used wherever possible in Glk.
 */
 
/**
 * glsi32:
 *
 * A 32-bit signed integer type, rarely used.
 */

/**
 * GLK_MODULE_UNICODE:
 *
 * If this preprocessor symbol is defined, so are the core Unicode functions and
 * constants (see %gestalt_Unicode). If not, not.
 */

/**
 * GLK_MODULE_IMAGE:
 *
 * If you are writing a C program, there is an additional complication. A
 * library which does not support graphics may not implement the graphics
 * functions at all. Even if you put gestalt tests around your graphics calls,
 * you may get link-time errors.
 * If the `glk.h` file is so old that it does not declare the graphics functions
 * and constants, you may even get compile-time errors.
 *
 * To avoid this, you can perform a preprocessor test for the existence of
 * %GLK_MODULE_IMAGE. If this is defined, so are all the functions and constants
 * described in this section. If not, not.
 *
 * <note><para>
 *   To be extremely specific, there are two ways this can happen. If the 
 *   <filename class="headerfile">glk.h</filename> file that comes with the
 *   library is too old to have the graphics declarations in it, it will of
 *   course lack %GLK_MODULE_IMAGE as well. If the <filename 
 *   class="headerfile">glk.h</filename> file is recent, but the library is old,
 *   the definition of %GLK_MODULE_IMAGE should be removed from <filename 
 *   class="headerfile">glk.h</filename>, to avoid link errors. This is not a
 *   great solution. A better one is for the library to implement the graphics
 *   functions as stubs that do nothing (or cause run-time errors). Since no
 *   program will call the stubs without testing %gestalt_Graphics, this is
 *   sufficient.
 * </para></note>
 */

/**
 * GLK_MODULE_SOUND2:
 *
 * If you are writing a C program, there is an additional complication. A 
 * library which does not support sound may not implement the sound functions at
 * all. Even if you put gestalt tests around your sound calls, you may get 
 * link-time errors.
 * If the `glk.h` file is so old that it does not declare the sound functions
 * and constants, you may even get compile-time errors.
 *
 * To avoid this, you can perform a preprocessor test for the existence of
 * %GLK_MODULE_SOUND2. If this is defined, so are all the functions and constants
 * described in this section. If not, not.
 */

/**
 * GLK_MODULE_SOUND:
 *
 * You can perform a preprocessor test for the existence of %GLK_MODULE_SOUND.
 * If this is defined, so are all the functions and constants described in this
 * section. If not, not.
 */
 
/**
 * GLK_MODULE_HYPERLINKS:
 * 
 * If you are writing a C program, you can perform a preprocessor test for the
 * existence of %GLK_MODULE_HYPERLINKS. If this is defined, so are all the
 * functions and constants described in this section. If not, not.
 */

/**
 * GLK_MODULE_UNICODE_NORM:
 *
 * If this preprocessor symbol is defined, so are the Unicode normalization
 * functions (see %gestalt_UnicodeNorm). If not, not.
 */

/**
 * GLK_MODULE_DATETIME:
 *
 * If you are writing a C program, you can perform a preprocessor test for the
 * existence of %GLK_MODULE_DATETIME. If this is defined, so are all the
 * functions and data types described in this section.
 */

/**
 * GLK_MODULE_LINE_ECHO:
 *
 * If this preprocessor symbol is defined, so is glk_set_echo_line_event(). If
 * not, not.
 */

/**
 * GLK_MODULE_LINE_TERMINATORS:
 *
 * If this preprocessor symbol is defined, so is
 * glk_set_terminators_line_event(). If not, not.
 */

/**
 * GLK_MODULE_RESOURCE_STREAM:
 *
 * If this preprocessor symbol is defined, so are glk_stream_open_resource() and
 * glk_stream_open_resource_uni().
 * If not, not.
 */

/**
 * winid_t:
 *
 * Opaque structure representing a Glk window. It has no user-accessible 
 * members.
 */
 
/**
 * strid_t:
 *
 * Opaque structure representing an input or output stream. It has no
 * user-accessible members.
 */
 
/**
 * frefid_t:
 * 
 * Opaque structure representing a file reference. It has no user-accessible
 * members.
 */

/**
 * schanid_t:
 * 
 * Opaque structure representing a sound channel. It has no user-accessible
 * members.
 */
  
/**
 * gestalt_Version:
 *
 * For an example of the gestalt mechanism, consider the selector
 * %gestalt_Version. If you do
 * |[<!--language="C"-->
 * glui32 res;
 * res = glk_gestalt(gestalt_Version, 0);
 * ]|
 * `res` will be set to a 32-bit number which encodes the version of the Glk
 * spec which the library implements.
 * The upper 16 bits stores the major version number; the next 8 bits stores the
 * minor version number; the low 8 bits stores an even more minor version
 * number, if any.
 *
 * <note><para>
 *   So the version number 78.2.11 would be encoded as 0x004E020B.
 * </para></note>
 *
 * The current Glk specification version is 0.7.5, so this selector will return
 * 0x00000705.
 *
 * |[<!--language="C"-->
 * glui32 res;
 * res = glk_gestalt_ext(gestalt_Version, 0, NULL, 0);
 * ]|
 * does exactly the same thing. Note that, in either case, the second argument 
 * is not used; so you should always pass 0 to avoid future surprises.
 */

/**
 * gestalt_CharInput:
 *
 * If you set `ch` to a character code, or a special code (from 0xFFFFFFFF
 * down), and call
 * |[<!--language="C"-->
 * glui32 res;
 * res = glk_gestalt(gestalt_CharInput, ch);
 * ]|
 * then `res` will be %TRUE (1) if that character can be typed by the player in
 * character input, and %FALSE (0) if not.
 * See [Character Input][chimara-Character-Input].
 */

/**
 * gestalt_LineInput:
 *
 * If you set `ch` to a character code, and call
 * |[<!--language="C"-->
 * glui32 res;
 * res = glk_gestalt(gestalt_LineInput, ch);
 * ]|
 * then `res` will be %TRUE (1) if that character can be typed by the player in
 * line input, and %FALSE (0) if not.
 * Note that if `ch` is a nonprintable Latin-1 character (0 to 31, 127 to 159),
 * then this is guaranteed to return %FALSE.
 * See [Line Input][chimara-Line-Input].
 */

/**
 * gestalt_CharOutput:
 *
 * If you set `ch` to a character code (Latin-1 or higher), and call
 * |[<!--language="C"-->
 * glui32 res, len;
 * res = glk_gestalt_ext(gestalt_CharOutput, ch, &amp;len, 1);
 * ]|
 * then `res` will be one of %gestalt_CharOutput_CannotPrint,
 * %gestalt_CharOutput_ExactPrint, or %gestalt_CharOutput_ApproxPrint (see 
 * below.)
 * 
 * In all cases, `len` (the #glui32 value pointed at by the third argument) will
 * be the number of actual glyphs which will be used to represent the character.
 * In the case of %gestalt_CharOutput_ExactPrint, this will always be 1; for
 * %gestalt_CharOutput_CannotPrint, it may be 0 (nothing printed) or higher; for
 * %gestalt_CharOutput_ApproxPrint, it may be 1 or higher.
 * This information may be useful when printing text in a fixed-width font.
 *
 * <note><para>
 *   As described in <link linkend="chimara-Other-API-Conventions">Other API
 *   Conventions</link>, you may skip this information by passing %NULL as the
 *   third argument in glk_gestalt_ext(), or by calling glk_gestalt() instead.
 * </para></note>
 *
 * This selector will always return %gestalt_CharOutput_CannotPrint if `ch` is
 * an unprintable eight-bit character (0 to 9, 11 to 31, 127 to 159.)
 *
 * <note><para>
 *   Make sure you do not get confused by signed byte values. If you set a
 *   “<type>signed char</type>” variable <code>ch</code> to 0xFE, the
 *   small-thorn character (&thorn;), it will wind up as -2.
 *   (The same is true of a “<type>char</type>” variable, if your compiler
 *   treats “<type>char</type>” as signed!)
 *   If you then call
 *   |[
 *   res = glk_gestalt(gestalt_CharOutput, ch);
 *   ]|
 *   then (by the definition of C/C++) <code>ch</code> will be sign-extended to
 *   0xFFFFFFFE, which is not a legitimate character, even in Unicode. You
 *   should write
 *   |[
 *   res = glk_gestalt(gestalt_CharOutput, (unsigned char)ch);
 *   ]|
 *   instead.
 * </para></note>
 * <note><para>
 *   Unicode includes the concept of non-spacing or combining characters, which 
 *   do not represent glyphs; and double-width characters, whose glyphs take up
 *   two spaces in a fixed-width font. Future versions of this spec may 
 *   recognize these concepts by returning a <code>len</code> of 0 or 2 when
 *   %gestalt_CharOutput_ExactPrint is used. For the moment, we are adhering to 
 *   a policy of “simple stuff first”.
 * </para></note>
 */
 
/**
 * gestalt_CharOutput_CannotPrint:
 *
 * When the %gestalt_CharOutput selector returns this for a character, the
 * character cannot be meaningfully printed. If you try, the player may see
 * nothing, or may see a placeholder.
 */

/**
 * gestalt_CharOutput_ApproxPrint:
 *
 * When the %gestalt_CharOutput selector returns this for a character, the 
 * library will print some approximation of the character. It will be more or 
 * less right, but it may not be precise, and it may not be distinguishable from
 * other, similar characters.
 * (Examples: “`ae`” for the one-character “&aelig;” ligature, “`e`” for
 * “&egrave;”, “`|`” for a broken vertical bar (&brvbar;).)
 */
 
/**
 * gestalt_CharOutput_ExactPrint:
 *
 * When the %gestalt_CharOutput selector returns this for a character, the
 * character will be printed exactly as defined.
 */

/**
 * gestalt_MouseInput:
 *
 * You can test whether mouse input is supported with the %gestalt_MouseInput 
 * selector.
 * |[<!--language="C"-->
 * res = glk_gestalt(gestalt_MouseInput, windowtype);
 * ]|
 * This will return %TRUE (1) if windows of the given type support mouse input.
 * If this returns %FALSE (0), it is still legal to call
 * glk_request_mouse_event(), but it will have no effect, and you will never get
 * mouse events.
 */ 
 
/**
 * gestalt_Timer:
 *
 * You can test whether the library supports timer events:
 * |[<!--language="C"-->
 * res = glk_gestalt(gestalt_Timer, 0);
 * ]|
 * This returns %TRUE (1) if timer events are supported, and %FALSE (0) if they 
 * are not.
 */

/**
 * gestalt_Graphics:
 * 
 * Before calling Glk graphics functions, you should use the following gestalt
 * selector:
 * |[<!--language="C"-->
 * glui32 res;
 * res = glk_gestalt(gestalt_Graphics, 0);
 * ]|
 * This returns 1 if the overall suite of graphics functions is available. This
 * includes glk_image_draw(), glk_image_draw_scaled(), glk_image_get_info(),
 * glk_window_erase_rect(), glk_window_fill_rect(),
 * glk_window_set_background_color(), and glk_window_flow_break(). It also
 * includes the capability to create graphics windows.
 * 
 * If this selector returns 0, you should not try to call these functions. They
 * may have no effect, or they may cause a run-time error. If you try to create
 * a graphics window, you will get %NULL. 
 */

/**
 * gestalt_DrawImage:
 * 
 * This selector returns 1 if images can be drawn in windows of the given type. 
 * If it returns 0, glk_image_draw() will fail and return %FALSE (0). You should 
 * test %wintype_Graphics and %wintype_TextBuffer separately, since libraries 
 * may implement both, neither, or only one.  
 */

/**
 * gestalt_Sound2:
 *
 * You can test whether the library supports sound:
 * |[<!--language="C"-->
 * glui32 res;
 * res = glk_gestalt(gestalt_Sound2, 0);
 * ]|
 * This returns 1 if the overall suite of sound functions is available. This
 * includes all the functions defined in [this chapter][chimara-glk-spec-sound].
 * It also includes the capabilities described below under %gestalt_SoundMusic,
 * %gestalt_SoundVolume, and %gestalt_SoundNotify.
 */

/**
 * gestalt_Sound:
 *
 * |[<!--language="C"-->
 * glui32 res;
 * res = glk_gestalt(gestalt_Sound, 0);
 * ]|
 * This returns 1 if the older (pre-0.7.3) suite of sound functions is
 * available.
 * This includes glk_schannel_create(), glk_schannel_destroy(),
 * glk_schannel_iterate(), glk_schannel_get_rock(), glk_schannel_play(),
 * glk_schannel_play_ext(), glk_schannel_stop(), glk_schannel_set_volume(), and
 * glk_sound_load_hint().
 *
 * If this selector returns 0, you should not try to call these functions. They 
 * may have no effect, or they may cause a run-time error.
 *
 * This selector is guaranteed to return 1 if %gestalt_Sound2 does.
 */

/**
 * gestalt_SoundVolume:
 *
 * You can test whether the library supports setting the volume of sound 
 * channels: 
 * |[<!--language="C"-->
 * res = glk_gestalt(gestalt_SoundVolume, 0);
 * ]|
 * This selector returns 1 if the glk_schannel_set_volume() function works. If 
 * it returns zero, glk_schannel_set_volume() has no effect.
 *
 * This selector is guaranteed to return 1 if %gestalt_Sound2 does.
 */

/**
 * gestalt_SoundNotify:
 *
 * You can test whether the library supports sound notification events:
 * |[<!--language="C"-->
 * res = glk_gestalt(gestalt_SoundNotify, 0);
 * ]|
 * This selector returns 1 if the library supports sound notification events. If
 * it returns zero, you will never get such events.
 *
 * This selector is guaranteed to return 1 if %gestalt_Sound2 does.
 */

/**
 * gestalt_Hyperlinks:
 *
 * You can test whether the library supports hyperlinks:
 * |[<!--language="C"-->
 * glui32 res;
 * res = glk_gestalt(gestalt_Hyperlinks, 0); 
 * ]|
 * This returns 1 if the overall suite of hyperlinks functions is available.
 * This includes glk_set_hyperlink(), glk_set_hyperlink_stream(),
 * glk_request_hyperlink_event(), glk_cancel_hyperlink_event().
 *
 * If this selector returns 0, you should not try to call these functions. They
 * may have no effect, or they may cause a run-time error.
 */

/**
 * gestalt_HyperlinkInput:
 *
 * You can test whether hyperlinks are supported with the 
 * %gestalt_HyperlinkInput selector:
 * |[<!--language="C"-->
 * res = glk_gestalt(gestalt_HyperlinkInput, windowtype);
 * ]|
 * This will return %TRUE (1) if windows of the given type support hyperlinks.
 * If this returns %FALSE (0), it is still legal to call glk_set_hyperlink() and
 * glk_request_hyperlink_event(), but they will have no effect, and you will
 * never get hyperlink events.
 */

/** 
 * gestalt_SoundMusic:
 *
 * You can test whether music resources are supported:
 * |[<!--language="C"-->
 * res = glk_gestalt(gestalt_SoundMusic, 0);
 * ]|
 * This returns 1 if the library is capable of playing music sound resources. If 
 * it returns 0, only sampled sounds can be played.
 * <note><para>
 *   “Music sound resources” means MOD songs &mdash; the only music format that
 *   Blorb currently supports.
 *   The presence of this selector is, of course, an ugly hack.
 *   It is a concession to the current state of the Glk libraries, some of which
 *   can handle AIFF but not MOD sounds.
 * </para></note>
 *
 * This selector is guaranteed to return 1 if %gestalt_Sound2 does.
 */ 
  
/**
 * gestalt_GraphicsTransparency:
 *
 * This returns 1 if images with alpha channels can actually be drawn with the
 * appropriate degree of transparency. If it returns 0, the alpha channel is
 * ignored; fully transparent areas will be drawn in an implementation-defined
 * color.
 * <note><para>
 *   The JPEG format does not support transparency or alpha channels; the PNG 
 *   format does.
 * </para></note>
 */

/**
 * gestalt_GraphicsCharInput:
 *
 * This returns 1 if graphics windows can accept character input requests.
 * If it returns zero, do not call glk_request_char_event() or
 * glk_request_char_event_uni() on a graphics window.
 */

/**
 * gestalt_Unicode:
 *
 * The basic text functions will be available in every Glk library. The Unicode
 * functions may or may not be available. Before calling them, you should use
 * the %gestalt_Unicode and %gestalt_UnicodeNorm gestalt selectors.
 *
 * |[<!--language="C"-->
 * glui32 res;
 * res = glk_gestalt(gestalt_Unicode, 0);
 * ]|
 * This returns 1 if the core Unicode functions are available. If it returns 0,
 * you should not try to call them. They may print nothing, print gibberish, or
 * cause a run-time error. The Unicode functions include
 * glk_buffer_to_lower_case_uni(), glk_buffer_to_upper_case_uni(),  
 * glk_buffer_to_title_case_uni(), glk_put_char_uni(), glk_put_string_uni(),
 * glk_put_buffer_uni(), glk_put_char_stream_uni(), glk_put_string_stream_uni(),
 * glk_put_buffer_stream_uni(), glk_get_char_stream_uni(),
 * glk_get_buffer_stream_uni(), glk_get_line_stream_uni(),
 * glk_request_char_event_uni(), glk_request_line_event_uni(),
 * glk_stream_open_file_uni(), glk_stream_open_memory_uni().
 * 
 * If you are writing a C program, there is an additional complication. A
 * library which does not support Unicode may not implement the Unicode
 * functions at all. Even if you put gestalt tests around your Unicode calls,
 * you may get link-time errors.
 * If the `glk.h` file is so old that it does not declare the Unicode functions
 * and constants, you may even get compile-time errors.
 *
 * To avoid this, you can perform a preprocessor test for the existence of
 * %GLK_MODULE_UNICODE.
 */

/**
 * gestalt_UnicodeNorm:
 *
 * |[<!--language="C"-->
 * glui32 res;
 * res = glk_gestalt(gestalt_UnicodeNorm, 0);
 * ]|
 * This code returns 1 if the Unicode normalization functions are available. If
 * it returns 0, you should not try to call them. The Unicode normalization
 * functions include glk_buffer_canon_decompose_uni() and
 * glk_buffer_canon_normalize_uni().
 *
 * The equivalent preprocessor test for these functions is
 * %GLK_MODULE_UNICODE_NORM.
 */

/**
 * gestalt_LineInputEcho:
 *
 * |[<!--language="C"-->
 * res = glk_gestalt(gestalt_LineInputEcho, 0);
 * ]|
 *
 * This returns 1 if glk_set_echo_line_event() is supported, and 0 if it is not.
 * <note><para>
 *   Remember that if it is not supported, the behavior is always the default,
 *   which is line echoing <emphasis>enabled</emphasis>.
 * </para></note>
 */

/**
 * gestalt_LineTerminators:
 *
 * |[<!--language="C"-->
 * res = glk_gestalt(gestalt_LineTerminators, 0);
 * ]|
 *
 * This returns 1 if glk_set_terminators_line_event() is supported, and 0 if it
 * is not.
 */

/**
 * gestalt_LineTerminatorKey:
 *
 * |[<!--language="C"-->
 * res = glk_gestalt(gestalt_LineTerminatorKey, ch);
 * ]|
 *
 * This returns 1 if the keycode @ch can be passed to
 * glk_set_terminators_line_event(). If it returns 0, that keycode will be
 * ignored as a line terminator. Printable characters and %keycode_Return will
 * always return 0.
 */

/**
 * gestalt_DateTime:
 *
 * |[<!--language="C"-->
 * res = glk_gestalt(gestalt_DateTime, 0);
 * ]|
 *
 * This returns 1 if the overall suite of system clock functions, as described
 * in [this chapter][chimara-The-System-Clock], is available.
 *
 * If this selector returns 0, you should not try to call these functions. They
 * may have no effect, or they may cause a run-time error.
 *
 * <note><para>
 *   Glk timer events are covered by a different selector. See %gestalt_Timer.
 * </para></note>
 */

/**
 * gestalt_ResourceStream:
 *
 * |[<!--language="C"-->
 * res = glk_gestalt(gestalt_ResourceStream, 0);
 * ]|
 *
 * This returns 1 if the glk_stream_open_resource() and
 * glk_stream_open_resource_uni() functions are available. If it returns 0, you
 * should not call them.
 */

/**
 * evtype_None:
 *
 * No event. This is a placeholder, and glk_select() never returns it.
 */

/**
 * evtype_Timer:
 *
 * An event that repeats at fixed intervals.
 * See [Timer Events][chimara-Timer-Events].
 */
 
/**
 * evtype_CharInput:
 *
 * A keystroke event in a window.
 * See [Character Input Events][chimara-Character-Input-Events].
 *
 * If a window has a pending request for character input, and the player hits a
 * key in that window, glk_select() will return an event whose type is
 * %evtype_CharInput. Once this happens, the request is complete; it is no 
 * longer pending. You must call glk_request_char_event() or
 * glk_request_char_event_uni() if you want another character from that window.
 * 
 * In the event structure, @win tells what window the event came from. @val1 
 * tells what character was entered; this will be a character code, or a special
 * keycode.
 * (See [Character Input][chimara-Character-Input].)
 * If you called glk_request_char_event(), @val1 will be in 0..255, or else a
 * special keycode.
 * In any case, @val2 will be 0.
 */

/**
 * evtype_LineInput:
 *
 * A full line of input completed in a window.
 * See [Line Input Events][chimara-Line-Input-Events].
 *
 * If a window has a pending request for line input, the player can generally
 * hit the <keycap>enter</keycap> key (in that window) to complete line input.
 * The details will depend on the platform's native user interface.
 *
 * When line input is completed, glk_select() will return an event whose type is
 * %evtype_LineInput. Once this happens, the request is complete; it is no
 * longer pending. You must call glk_request_line_event() if you want another
 * line of text from that window.
 *
 * In the event structure, @win tells what window the event came from. @val1
 * tells how many characters were entered. @val2 will be 0 unless input was
 * ended by a special terminator key, in which case @val2 will be the keycode
 * (one of the values passed to glk_set_terminators_line_event()).
 *
 * The characters themselves are stored in the buffer specified in the original
 * glk_request_line_event() or glk_request_line_event_uni() call.
 *
 * <note><para>
 *   There is no null terminator or newline stored in the buffer.
 * </para></note>
 * 
 * It is illegal to print anything to a window which has line input pending. 
 *
 * <note><para>
 *   This is because the window may be displaying and editing the player's 
 *   input, and printing anything would make life unnecessarily complicated for
 *   the library.
 * </para></note>
 */

/**
 * evtype_MouseInput:
 *
 * A mouse click in a window.
 * See [Mouse Input Events][chimara-Mouse-Input-Events].
 */
 
/**
 * evtype_Arrange:
 *
 * An event signalling that the sizes of some windows have changed. 
 * 
 * Some platforms allow the player to resize the Glk window during play. This 
 * will naturally change the sizes of your windows. If this occurs, then
 * immediately after all the rearrangement, glk_select() will return an event
 * whose type is %evtype_Arrange. You can use this notification to redisplay the
 * contents of a graphics or text grid window whose size has changed.
 *
 * <note><para>
 *   The display of a text buffer window is entirely up to the library, so you
 *   don't need to worry about those.
 * </para></note>
 * 
 * In the event structure, @win will be %NULL if all windows are affected. If 
 * only some windows are affected, @win will refer to a window which contains 
 * all the affected windows. @val1 and @val2 will be 0.
 *
 * <note><para>
 *   You can always play it safe, ignore @win, and redraw every graphics and 
 *   text grid window.
 * </para></note>
 *
 * An arrangement event is guaranteed to occur whenever the player causes any
 * window to change size, as measured by its own metric. 
 *
 * <note><para>
 *   Size changes caused by you &mdash; for example, if you open, close, or 
 *   resize a window &mdash; do not trigger arrangement events. You must be 
 *   aware of the effects of your window management, and redraw the windows that
 *   you affect.
 * </para></note>
 * 
 * <note><para>
 *   It is possible that several different player actions can cause windows to
 *   change size. For example, if the player changes the screen resolution, an
 *   arrangement event might be triggered. This might also happen if the player
 *   changes his display font to a different size; the windows would then be
 *   different “sizes” in the metric of rows and columns, which is the important
 *   metric and the only one you have access to.
 * </para></note>
 * 
 * Arrangement events, like timer events, can be returned by glk_select_poll().
 * But this will not occur on all platforms. You must be ready to receive an
 * arrangement event when you call glk_select_poll(), but it is possible that it
 * will not arrive until the next time you call glk_select(). 
 *
 * <note><para>
 *   This is because on some platforms, window resizing is handled as part of
 *   player input; on others, it can be triggered by an external process such as 
 *   a window manager.
 * </para></note>
 */

/**
 * evtype_Redraw:
 *
 * An event signalling that graphics windows must be redrawn.
 *
 * On platforms that support graphics, it is possible that the contents of a
 * graphics window will be lost, and have to be redrawn from scratch. If this
 * occurs, then glk_select() will return an event whose type is %evtype_Redraw.
 *
 * In the event structure, @win will be %NULL if all windows are affected. If 
 * only some windows are affected, @win will refer to a window which contains 
 * all the affected windows. @val1 and @val2 will be 0.
 *
 * <note><para>
 *   You can always play it safe, ignore @win, and redraw every graphics window.
 * </para></note>
 *
 * Affected windows are already cleared to their background color when you 
 * receive the redraw event.
 * 
 * Redraw events can be returned by glk_select_poll(). But, like arrangement
 * events, this is platform-dependent. See %evtype_Arrange.
 *
 * For more about redraw events and how they affect graphics windows, see
 * [Graphics Windows][wintype-Graphics].
 */

/**
 * evtype_SoundNotify:
 *
 * The completion of a sound being played in a sound channel.
 *
 * On platforms that support sound, you can request to receive an 
 * %evtype_SoundNotify event when a sound finishes playing.
 * See [Playing Sounds][chimara-Playing-Sounds].
 */
 
/**
 * evtype_Hyperlink:
 *
 * The selection of a hyperlink in a window.
 * 
 * On platforms that support hyperlinks, you can request to receive an
 * %evtype_Hyperlink event when the player selects a link.
 * See [Accepting Hyperlink Events][chimara-Accepting-Hyperlink-Events].
 */

/**
 * evtype_VolumeNotify:
 *
 * The completion of a gradual volume change in a sound channel.
 *
 * On platforms that support sound, you can request to receive an
 * %evtype_VolumeNotify event when a gradual volume change completes.
 * See [Playing Sounds][chimara-Playing-Sounds].
 */

/**
 * event_t:
 * @type: the event type
 * @win: the window that spawned the event, or %NULL
 * @val1: information, the meaning of which depends on the type of event
 * @val2: more information, the meaning of which depends on the type of event
 *
 * The event structure is self-explanatory. @type is the event type. The window
 * that spawned the event, if relevant, is in @win. The remaining fields contain
 * more information specific to the event.
 *
 * The event types are described below. Note that %evtype_None is zero, and the
 * other values are positive. Negative event types (0x80000000 to 0xFFFFFFFF) 
 * are reserved for implementation-defined events. 
 */

/**
 * keycode_Unknown:
 *
 * Represents any key that has no Latin-1 or special code.
 */

/**
 * keycode_Left:
 *
 * Represents the <keycap function="left">left arrow</keycap> key.
 */
 
/**
 * keycode_Right:
 *
 * Represents the <keycap function="right">right arrow</keycap> key.
 */
 
/**
 * keycode_Up:
 *
 * Represents the <keycap function="up">up arrow</keycap> key.
 */
 
/**
 * keycode_Down:
 *
 * Represents the <keycap function="down">down arrow</keycap> key.
 */
 
/**
 * keycode_Return:
 *
 * Represents the <keycap function="enter">return</keycap> or <keycap 
 * function="enter">enter</keycap> keys.
 */

/**
 * keycode_Delete:
 *
 * Represents the <keycap function="delete">delete</keycap> or <keycap
 * function="backspace">backspace</keycap> keys.
 */
 
/**
 * keycode_Escape:
 *
 * Represents the <keycap function="escape">escape</keycap> key.
 */
 
/**
 * keycode_Tab:
 *
 * Represents the <keycap function="tab">tab</keycap> key.
 */

/**
 * keycode_PageUp:
 *
 * Represents the <keycap function="pageup">page up</keycap> key.
 */

/**
 * keycode_PageDown:
 *
 * Represents the <keycap function="pagedown">page down</keycap> key.
 */

/**
 * keycode_Home:
 *
 * Represents the <keycap function="home">home</keycap> key.
 */
 
/**
 * keycode_End:
 *
 * Represents the <keycap function="end">end</keycap> key.
 */

/**
 * keycode_Func1:
 *
 * Represents the <keycap>F1</keycap> key.
 */
 
/**
 * keycode_Func2:
 *
 * Represents the <keycap>F2</keycap> key.
 */

/**
 * keycode_Func3:
 *
 * Represents the <keycap>F3</keycap> key.
 */

/**
 * keycode_Func4:
 *
 * Represents the <keycap>F4</keycap> key.
 */

/**
 * keycode_Func5:
 *
 * Represents the <keycap>F5</keycap> key.
 */
 
/**
 * keycode_Func6:
 *
 * Represents the <keycap>F6</keycap> key.
 */

/**
 * keycode_Func7:
 *
 * Represents the <keycap>F7</keycap> key.
 */

/**
 * keycode_Func8:
 *
 * Represents the <keycap>F8</keycap> key.
 */

/**
 * keycode_Func9:
 *
 * Represents the <keycap>F9</keycap> key.
 */

/**
 * keycode_Func10:
 *
 * Represents the <keycap>F10</keycap> key.
 */

/**
 * keycode_Func11:
 *
 * Represents the <keycap>F11</keycap> key.
 */

/**
 * keycode_Func12:
 *
 * Represents the <keycap>F12</keycap> key.
 */

/**
 * style_Normal: 
 *
 * The style of normal or body text. A new window or stream always starts with
 * %style_Normal as the current style.
 */

/**
 * style_Emphasized: 
 *
 * Text which is emphasized.
 */

/**
 * style_Preformatted: 
 *
 * Text which has a particular arrangement of characters.
 * <note><para>
 *  This style, unlike the others, does have a standard appearance; it will 
 *  always be a fixed-width font. This is a concession to practicality. Games 
 *  often want to display maps or diagrams using character graphics, and this is
 *  the style for that.
 * </para></note>
 */
 
/**
 * style_Header: 
 * 
 * Text which introduces a large section. This is suitable for the title of an 
 * entire game, or a major division such as a chapter.
 */

/**
 * style_Subheader: 
 * 
 * Text which introduces a smaller section within a large section. 
 * <note><para>
 *  In a Colossal-Cave-style game, this is suitable for the name of a room (when
 *  the player looks around.)
 * </para></note>
 */

/**
 * style_Alert: 
 *
 * Text which warns of a dangerous condition, or one which the player should pay
 * attention to.
 */

/**
 * style_Note: 
 *
 * Text which notifies of an interesting condition.
 * <note><para>
 *  This is suitable for noting that the player's score has changed.
 * </para></note>
 */

/**
 * style_BlockQuote: 
 *
 * Text which forms a quotation or otherwise abstracted text.
 */

/**
 * style_Input: 
 *
 * Text which the player has entered. You should generally not use this style at
 * all; the library uses it for text which is typed during a line-input request.
 * One case when it is appropriate for you to use %style_Input is when you are 
 * simulating player input by reading commands from a text file.
 */

/**
 * style_User1: 
 * 
 * This style has no particular semantic meaning. You may define a meaning 
 * relevant to your own work, and use it as you see fit.
 */

/**
 * style_User2: 
 *
 * Another style available for your use. 
 */

/**
 * stream_result_t:
 * @readcount: Number of characters read from the stream.
 * @writecount: Number of characters printed to the stream, including ones that
 * were thrown away.
 *
 * If you are interested in the character counts of a stream (see
 * [Streams][chimara-Streams]), then you can pass a pointer to #stream_result_t
 * as an argument of glk_stream_close() or glk_window_close().
 * The structure will be filled with the stream's final character counts.
 */

/**
 * wintype_AllTypes:
 *
 * A constant representing all window types, which may be used as the @wintype
 * argument in glk_stylehint_set().
 */

/** 
 * wintype_Pair:
 * 
 * A pair window is completely filled by the two windows it contains. It
 * supports no input and no output, and it has no size.
 * 
 * You cannot directly create a pair window; one is automatically created
 * every time you split a window with glk_window_open(). Pair windows are
 * always created with a rock value of 0.
 * 
 * You can close a pair window with glk_window_close(); this also closes every
 * window contained within the pair window.
 * 
 * It is legal to split a pair window when you call glk_window_open().
 */
 
/**
 * wintype_Blank:
 * 
 * A blank window is always blank. It supports no input and no output. (You
 * can call glk_window_get_stream() on it, as you can with any window, but
 * printing to the resulting stream has no effect.) A blank window has no
 * size; glk_window_get_size() will return (0,0), and it is illegal to set a
 * window split with a fixed size in the measurement system of a blank window.
 * 
 * <note><para>
 *   A blank window is not the same as there being no windows. When Glk starts
 *   up, there are no windows at all, not even a window of the blank type.
 * </para></note>
 */
 
/**
 * wintype_TextBuffer: 
 *
 * A text buffer window contains a linear stream of text. It supports output;
 * when you print to it, the new text is added to the end. There is no way for
 * you to affect text which has already been printed. There are no guarantees
 * about how much text the window keeps; old text may be stored forever, so
 * that the user can scroll back to it, or it may be thrown away as soon as it
 * scrolls out of the window. 
 * 
 * <note><para>
 *   Therefore, there may or may not be a player-controllable scroll bar or
 *   other scrolling widget.
 * </para></note>
 * 
 * The display of the text in a text buffer is up to the library. Lines will
 * probably not be broken in the middles of words &mdash; but if they are, the
 * library is not doing anything illegal, only ugly. Text selection and copying
 * to a clipboard, if available, are handled however is best on the player's
 * machine. Paragraphs (as defined by newline characters in the output) may be
 * indented. 
 * 
 * <note><para>
 *   You should not, in general, fake this by printing spaces before each
 *   paragraph of prose text. Let the library and player preferences handle
 *   that. Special cases (like indented lists) are of course up to you.
 * </para></note>
 * 
 * When a text buffer is cleared (with glk_window_clear()), the library will do
 * something appropriate; the details may vary. It may clear the window, with
 * later text appearing at the top &mdash; or the bottom. It may simply print
 * enough blank lines to scroll the current text out of the window. It may
 * display a distinctive page-break symbol or divider.
 * 
 * The size of a text buffer window is necessarily imprecise. Calling
 * glk_window_get_size() will return the number of rows and columns that would
 * be available <emphasis>if</emphasis> the window was filled with 
 * “0” (zero) characters in the “normal” font.
 * However, the window may use a non-fixed-width font, so that number of
 * characters in a line could vary. The window might even support 
 * variable-height text (say, if the player is using large text for emphasis);
 * that would make the number of lines in the window vary as well.
 * 
 * Similarly, when you set a fixed-size split in the measurement system of a
 * text buffer, you are setting a window which can handle a fixed number of rows
 * (or columns) of “0” characters.
 * The number of rows (or characters) that will actually be displayed depends on
 * font variances.
 *
 * A text buffer window supports both character and line input, but not mouse
 * input.
 * 
 * In character input, there will be some visible signal that the window is
 * waiting for a keystroke. (Typically, a cursor at the end of the text.) When
 * the player hits a key in that window, an event is generated, but the key is
 * <emphasis>not</emphasis> printed in the window.
 * 
 * In line input, again, there will be some visible signal. It is most common
 * for the player to compose input in the window itself, at the end of the text.
 * (This is how IF story input usually looks.) But it's not strictly required.
 * An alternative approach is the way MUD clients usually work: there is a
 * dedicated one-line input window, outside of Glk's window space, and the user
 * composes input there.
 * 
 * <note><para>
 *   If this approach is used, there will still be some way to handle input from
 *   two windows at once. It is the library's responsibility to make this
 *   available to the player. You only need request line input and  wait for the
 *   result.
 * </para></note>
 * 
 * By default, when the player finishes his line of input, the library will
 * display the input text at the end of the buffer text (if it wasn't there
 * already.) It will be followed by a newline, so that the next text you print
 * will start a new line (paragraph) after the input.
 * 
 * If you call glk_cancel_line_event(), the same thing happens; whatever text
 * the user was composing is visible at the end of the buffer text, followed by
 * a newline.
 *
 * However, this default behavior can be changed with the
 * glk_set_echo_line_event() call. If the default echoing is disabled, the
 * library will <emphasis>not</emphasis> display the input text (plus newline)
 * after input is either completed or cancelled. The buffer will end with
 * whatever prompt you displayed before requesting input. If you want the
 * traditional input behavior, it is then your responsibility to print the text,
 * using the Input text style, followed by a newline (in the original style).
 */
 
/**
 * wintype_TextGrid: 
 * 
 * A text grid contains a rectangular array of characters, in a fixed-width
 * font. Its size is the number of columns and rows of the array.
 * 
 * A text grid window supports output. It maintains knowledge of an output
 * cursor position. When the window is opened, it is filled with blanks (space
 * characters), and the output cursor starts in the top left corner &mdash;
 * character (0,0). If the window is cleared with glk_window_clear(), the window
 * is filled with blanks again, and the cursor returns to the top left corner.
 * 
 * When you print, the characters of the output are laid into the array in
 * order, left to right and top to bottom. When the cursor reaches the end of a
 * line, or if a newline (0x0A) is printed, the cursor goes to the beginning of
 * the next line. The library makes <emphasis>no</emphasis> attempt to wrap
 * lines at word breaks. If the cursor reaches the end of the last line, further
 * printing has no effect on the window until the cursor is moved.
 * 
 * <note><para>
 *   Note that printing fancy characters may cause the cursor to advance more
 *   than one position per character. (For example, the “&aelig;” ligature may
 *   print as two characters.)
 *   See <link linkend="chimara-Output">Output</link>, for how to test this
 *   situation.
 * </para></note>
 * 
 * You can set the cursor position with glk_window_move_cursor().
 * 
 * When a text grid window is resized smaller, the bottom or right area is
 * thrown away, but the remaining area stays unchanged. When it is resized
 * larger, the new bottom or right area is filled with blanks.
 * 
 * <note><para>
 *   You may wish to watch for %evtype_Arrange events, and clear-and-redraw your
 *   text grid windows when you see them change size.
 * </para></note>
 * 
 * Text grid window support character and line input, as well as mouse input (if
 * a mouse is available.)
 * 
 * Mouse input returns the position of the character that was touched, from
 * (0,0) to <inlineequation><alt>(width-1,height-1)</alt><mathphrase>(width - 1,
 * height - 1)</mathphrase></inlineequation>.
 *
 * Character input is as described in the previous section.
 * 
 * Line input is slightly different; it is guaranteed to take place in the
 * window, at the output cursor position. The player can compose input only to
 * the right edge of the window; therefore, the maximum input length
 * is <inlineequation><alt>(windowwidth - 1 -
 * cursorposition)</alt><mathphrase>(windowwidth - 1 -
 * cursorposition)</mathphrase></inlineequation>.
 * If the maxlen argument of glk_request_line_event() is smaller than this, the
 * library will not allow the input cursor to go more than maxlen characters
 * past its start point.
 *
 * <note><para>
 *   This allows you to enter text in a fixed-width field, without the player
 *   being able to overwrite other parts of the window.
 * </para></note>
 * 
 * When the player finishes his line of input, it will remain visible in the
 * window, and the output cursor will be positioned at the beginning of the 
 * <emphasis>next</emphasis> row. Again, if you glk_cancel_line_event(), the
 * same thing happens. The glk_set_echo_line_event() call has no effect in grid
 * windows.
 */
 
/**
 * wintype_Graphics: 
 * 
 * A graphics window contains a rectangular array of pixels. Its size is the
 * number of columns and rows of the array.
 * 
 * Each graphics window has a background color, which is initially white. You
 * can change this; see [Graphics in Graphics
 * Windows][chimara-Graphics-in-Graphics-Windows].
 *
 * When a graphics window is resized smaller, the bottom or right area is
 * thrown away, but the remaining area stays unchanged. When it is resized
 * larger, the new bottom or right area is filled with the background color.
 * 
 * <note><para>
 *   You may wish to watch for %evtype_Arrange events, and clear-and-redraw your
 *   graphics windows when you see them change size.
 * </para></note>
 * 
 * In some libraries, you can receive a graphics-redraw event (%evtype_Redraw)
 * at any time. This signifies that the window in question has been cleared to
 * its background color, and must be redrawn. If you create any graphics
 * windows, you <emphasis>must</emphasis> handle these events.
 * 
 * <note><para>
 *   Redraw events can be triggered when a Glk window is uncovered or made
 *   visible by the platform's window manager. On the other hand, some Glk
 *   libraries handle these problem automatically &mdash; for example, with a
 *   backing store &mdash; and do not send you redraw events. On the third hand,
 *   the backing store may be discarded if memory is low, or for other reasons
 *   &mdash; perhaps the screen's color depth has changed. So redraw events are
 *   always a possibility, even in clever libraries. This is why you must be
 *   prepared to handle them.
 * 
 *   However, you will not receive a redraw event when you create a graphics
 *   window. It is assumed that you will do the initial drawing of your own
 *   accord. You also do not get redraw events when a graphics window is
 *   enlarged. If you ordered the enlargement, you already know about it; if the
 *   player is responsible, you receive a window-arrangement event, which covers
 *   the situation.
 * </para></note>
 * 
 * For a description of the drawing functions that apply to graphics windows,
 * see [Graphics in Graphics Windows][chimara-Graphics-in-Graphics-Windows].
 *
 * Graphics windows do not support text output, nor line input.
 * They may support character input.
 *
 * <note><para>
 *   Character input for graphics windows was added in Glk spec 0.7.5.
 *   Older interpreters may not support this feature.
 * </para></note>
 *
 * Not all libraries support graphics windows. You can test whether Glk graphics
 * are available using the gestalt system. In a C program, you can also test
 * whether the graphics functions are defined at compile-time.
 * See [Testing for Graphics
 * Capabilities][chimara-Testing-for-Graphics-Capabilities].
 *
 * <note><para>
 *   As with all windows, you should also test for %NULL when you create a
 *   graphics window.
 * </para></note>
 */
 
/**
 * winmethod_Left:
 *
 * When calling glk_window_open() with this @method, the new window will be 
 * to the left of the old one which was split.
 */

/**
 * winmethod_Right:
 *
 * When calling glk_window_open() with this @method, the new window will be 
 * to the right of the old one which was split.
 */

/**
 * winmethod_Above:
 *
 * When calling glk_window_open() with this @method, the new window will be 
 * above the old one which was split.
 */
 
/**
 * winmethod_Below:
 *
 * When calling glk_window_open() with this @method, the new window will be 
 * below the old one which was split.
 */
 
/**
 * winmethod_Fixed:
 *
 * When calling glk_window_open() with this @method, the new window will be 
 * a fixed size. (See glk_window_open()).
 */

/**
 * winmethod_Proportional:
 *
 * When calling glk_window_open() with this @method, the new window will be 
 * a given proportion of the old window's size. (See glk_window_open()).
 */

/**
 * winmethod_Border:
 *
 * When calling glk_window_open() with this @method, it specifies that there
 * should be a visible window border between the new window and its sibling.
 * (This is a hint to the library.)
 *
 * > # Chimara #
 * > There will only be a visible border if the #ChimaraGlk:spacing property
 * > is nonzero.
 * > Setting #ChimaraGlk:spacing to zero disables all borders on Glk windows.
 */

/**
 * winmethod_NoBorder:
 *
 * When calling glk_window_open() with this @method, it specifies that there
 * should not be a visible window border between the new window and its sibling.
 * (This is a hint to the library; you might specify NoBorder between two
 * graphics windows that should form a single image.)
 */

/** 
 * fileusage_Data: 
 *
 * Any other kind of file (preferences, statistics, arbitrary data.) 
 */

/**
 * fileusage_SavedGame: 
 * 
 * A file which stores game state.
 */

/**
 * fileusage_Transcript: 
 * 
 * A file which contains a stream of text from the game (often an echo stream
 * from a window.)
 */
 
/** 
 * fileusage_InputRecord: 
 * 
 * A file which records player input.
 */

/** 
 * fileusage_TextMode: 
 *
 * The file contents will be transformed to a platform-native text file as they
 * are written out. Newlines may be converted to linefeeds or 
 * linefeed-plus-carriage-return combinations; Latin-1 characters may be
 * converted to native character codes. When reading a file in text mode, native
 * line breaks will be converted back to newline (0x0A) characters, and native
 * character codes may be converted to Latin-1 or UTF-8. 
 *
 * <note><para>
 *   Line breaks will always be converted; other conversions are more
 *   questionable. If you write out a file in text mode, and then read it back
 *   in text mode, high-bit characters (128 to 255) may be transformed or lost.
 * </para></note>
 *
 * > # Chimara #
 * > Text mode files in Chimara are in UTF-8, which is GTK+'s native file
 * > encoding.
 */

/**
 * fileusage_BinaryMode: 
 *
 * The file contents will be stored exactly as they are written, and read back
 * in the same way. The resulting file may not be viewable on platform-native
 * text file viewers.
 */

/**
 * fileusage_TypeMask:
 *
 * Bitwise AND this value with a file usage argument to find whether the file
 * type is %fileusage_SavedGame, %fileusage_Transcript, %fileusage_InputRecord,
 * or %fileusage_Data.
 */

/**
 * filemode_Write: 
 *
 * An output stream.
 *
 * <note><para>
 *   Corresponds to mode <code>"w"</code> in the stdio library, using fopen().
 * </para></note>
 */

/** 
 * filemode_Read: 
 *
 * An input stream.
 *
 * <note><para>
 *   Corresponds to mode <code>"r"</code> in the stdio library, using fopen().
 * </para></note>
 */

/**
 * filemode_ReadWrite: 
 *
 * Both an input and an output stream.
 *
 * <note><para>
 *   Corresponds to mode <code>"r+"</code> in the stdio library, using fopen().
 * </para></note>
 */

/**
 * filemode_WriteAppend: 
 *
 * An output stream, but the data will added to the end of whatever already
 * existed in the destination, instead of replacing it. 
 *
 * <note><para>
 *   Confusingly, %filemode_WriteAppend cannot be mode <code>"a"</code>, because
 *   the stdio spec says that when you open a file with mode <code>"a"</code>,
 *   then fseek() doesn't work. So we have to use mode <code>"r+"</code> for
 *   appending. Then we run into the <emphasis>other</emphasis> stdio problem,
 *   which is that <code>"r+"</code> never creates a new file. So
 *   %filemode_WriteAppend has to <emphasis>first</emphasis> open the file with
 *   <code>"a"</code>, close it, reopen with <code>"r+"</code>, and then fseek()
 *   to the end of the file. For %filemode_ReadWrite, the process is the same,
 *   except without the fseek() &mdash; we begin at the beginning of the file.
 * </para></note>
 */
 
/**
 * seekmode_Start:
 *
 * In glk_stream_set_position(), signifies that @pos is counted in characters
 * after the beginning of the file.
 */
 
/**
 * seekmode_Current: 
 *
 * In glk_stream_set_position(), signifies that @pos is counted in characters
 * after the current position (moving backwards if @pos is negative.)
 */

/** 
 * seekmode_End: 
 *
 * In glk_stream_set_position(), signifies that @pos is counted in characters
 * after the end of the file. (@pos should always be zero or negative, so that
 * this will move backwards to a  position within the file.
 */

/**
 * stylehint_Indentation: 
 *
 * How much to indent lines of text in the given style. May be a negative 
 * number, to shift the text out (left) instead of in (right). The exact metric
 * isn't precisely specified; you can assume that +1 is the smallest indentation
 * possible which is clearly visible to the player.
 */

/**
 * stylehint_ParaIndentation: 
 *
 * How much to indent the first line of each paragraph. This is in addition to 
 * the indentation specified by %stylehint_Indentation. This too may be 
 * negative, and is measured in the same units as %stylehint_Indentation.
 */

/**
 * stylehint_Justification: 
 *
 * The value of this hint must be one of the constants 
 * %stylehint_just_LeftFlush, %stylehint_just_LeftRight (full justification), 
 * %stylehint_just_Centered, or %stylehint_just_RightFlush.
 */

/** 
 * stylehint_Size: 
 *
 * How much to increase or decrease the font size. This is relative; 0 means the
 * interpreter's default font size will be used, positive numbers increase it, 
 * and negative numbers decrease it. Again, +1 is the smallest size increase 
 * which is easily visible. 
 * <note><para>
 *  The amount of this increase may not be constant. +1 might increase an 
 *  8-point font to 9-point, but a 16-point font to 18-point.
 * </para></note>
 */

/**
 * stylehint_Weight: 
 *
 * The value of this hint must be 1 for heavy-weight fonts (boldface), 0 for 
 * normal weight, and -1 for light-weight fonts.
 */

/**
 * stylehint_Oblique: 
 *
 * The value of this hint must be 1 for oblique fonts (italic), or 0 for normal
 * angle.
 */
 
/** 
 * stylehint_Proportional: 
 * 
 * The value of this hint must be 1 for proportional-width fonts, or 0 for 
 * fixed-width.
 */

/**
 * stylehint_TextColor: 
 * 
 * The foreground color of the text. This is encoded in the 32-bit hint value: 
 * the top 8 bits must be zero, the next 8 bits are the red value, the next 8 
 * bits are the green value, and the bottom 8 bits are the blue value. Color 
 * values range from 0 to 255. 
 * <note><para>
 *   So 0x00000000 is black, 0x00FFFFFF is white, and 0x00FF0000 is bright red.
 * </para></note>
 */
 
/** 
 * stylehint_BackColor: 
 *
 * The background color behind the text. This is encoded the same way as 
 * %stylehint_TextColor.
 */
 
/** 
 * stylehint_ReverseColor: 
 *
 * The value of this hint must be 0 for normal printing (%stylehint_TextColor on 
 * %stylehint_BackColor), or 1 for reverse printing (%stylehint_BackColor on 
 * %stylehint_TextColor). 
 * <note><para>
 *  Some libraries may support this hint but not the %stylehint_TextColor and 
 *  %stylehint_BackColor hints. Other libraries may take the opposite tack; 
 *  others may support both, or neither.
 * </para></note>
 */
 
/**
 * stylehint_just_LeftFlush:
 *
 * A value for %stylehint_Justification representing left-justified text.
 */ 
 
/**
 * stylehint_just_LeftRight:
 *
 * A value for %stylehint_Justification representing fully justified text.
 */ 
 
/**
 * stylehint_just_Centered:
 *
 * A value for %stylehint_Justification representing centered text.
 */ 
 
/**
 * stylehint_just_RightFlush:
 *
 * A value for %stylehint_Justification representing right-justified text.
 */

/**
 * imagealign_InlineUp:
 *
 * The image appears at the current point in the text, sticking up. That is, the
 * bottom edge of the image is aligned with the baseline of the line of text.
 */

/**
 * imagealign_InlineDown:
 *
 * The image appears at the current point, and the top edge is aligned with the
 * top of the line of text.
 */

/**
 * imagealign_InlineCenter:
 *
 * The image appears at the current point, and it is centered between the top
 * and baseline of the line of text. If the image is taller than the line of
 * text, it will stick up and down equally.
 */

/**
 * imagealign_MarginLeft:
 * 
 * The image appears in the left margin. Subsequent text will be displayed to
 * the right of the image, and will flow around it &mdash; that is, it will be
 * left-indented for as many lines as it takes to pass the image.
 *
 * <warning><para>Margin images are not implemented yet.</para></warning>
 */

/**
 * imagealign_MarginRight:
 *
 * The image appears in the right margin, and subsequent text will flow around
 * it on the left.
 *
 * <warning><para>Margin images are not implemented yet.</para></warning>
 */

/**
 * glkdate_t:
 * @year: The full (four-digit) year
 * @month: The month number, ranging from 1-12, 1 is January
 * @day: The day of the month, ranging from 1-31
 * @weekday: The day of the week, ranging from 0-6, 0 is Sunday
 * @hour: The hour of the day, ranging from 0-23
 * @minute: The minute of the hour, ranging from 0-59
 * @second: The second of the minute, ranging from 0-59; may be 60 during a leap
 * second
 * @microsec: The fraction of the second in microseconds, ranging from 0-999999
 *
 * This structure represents a human-readable date in a specific timezone.
 */

/**
 * glktimeval_t:
 * @high_sec: The most significant 32 bits of the timestamp in seconds.
 * @low_sec: The least significant 32 bits of the timestamp in seconds.
 * @microsec: The fraction of the timestamp, in microseconds, ranging from
 * 0-999999.
 *
 * This structure represents the Unix timestamp, i.e. the number of seconds
 * since January 1, 1970.
 */
 
/*---------- TYPES, FUNCTIONS AND CONSTANTS FROM GI_DISPA.H ------------------*/

/**
 * gidispatch_count_classes:
 * 
 * Returns the number of opaque object classes used by the library. You will
 * need to know this if you want to keep track of opaque objects as they are
 * created; see [Opaque Object Registry][gidispatch-set-object-registry].
 *
 * As of Glk API 0.7.0, there are four classes: windows, streams, filerefs, and
 * sound channels (numbered 0, 1, 2, and 3 respectively.)
 *
 * Returns: Number of opaque object classes used by the library.
 */

/**
 * gidispatch_get_class:
 * @index: Unique integer index of the class.
 *
 * Returns a structure describing an opaque class that the library exports.
 * @index can range from 0 to <inlineequation><mathphrase>N -
 * 1</mathphrase><alt>N - 1</alt></inlineequation>, where N is the value
 * returned by gidispatch_count_classes().
 *
 * Returns: A #gidispatch_intconst_t structure describing the class.
 */

/**
 * gidispatch_count_intconst:
 *
 * Returns the number of integer constants exported by the library.
 *
 * Returns: Number of integer constants exported by the library.
 */
 
/**
 * gidispatch_get_intconst:
 * @index: Unique integer index of the integer constant.
 *
 * Returns a structure describing an integer constant which the library exports.
 * These are, roughly, all the constants defined in the `glk.h` file.
 * @index can range from 0 to <inlineequation><mathphrase>N -
 * 1</mathphrase><alt>N - 1</alt></inlineequation>, where N is the value
 * returned by gidispatch_count_intconst().
 *
 * Returns: A #gidispatch_intconst_t structure describing the integer constant.
 */

/**
 * gidispatch_intconst_t:
 * @name: Symbolic name of the integer constant.
 * @val: Value of the integer constant.
 *
 * This structure simply contains a string and a value. The string is a
 * symbolic name of the value, and can be re-exported to anyone interested in
 * using Glk constants.
 *
 * <note><para>
 *   In the current <filename>gi_dispa.c</filename> library, these structures
 *   are static and immutable, and will never be deallocated. However, it is
 *   safer to assume that the structure may be reused in future
 *   gidispatch_get_intconst() calls.
 * </para></note>
 */
 
/**
 * gidispatch_count_functions:
 *
 * Returns the number of functions exported by the library.
 *
 * Returns: Number of functions exported by the library.
 */
 
/**
 * gidispatch_get_function:
 * @index: Unique integer index of the function.
 *
 * Returns a structure describing a Glk function.
 * @index can range from 0 to <inlineequation><mathphrase>N -
 * 1</mathphrase><alt>N - 1</alt></inlineequation>, where N is the value
 * returned by gidispatch_count_functions().
 *
 * <note><para>
 *   Again, it is safest to assume that the structure is only valid until the
 *   next gidispatch_get_function() or gidispatch_get_function_by_id() call.
 * </para></note>
 *
 * Returns: A #gidispatch_function_t structure describing the function.
 */
 
/**
 * gidispatch_function_t:
 * @id: Dispatch selector of the function.
 * @fnptr: Pointer to the function.
 * @name: Name of the function, without the `glk_` prefix.
 *
 * The @id field is a selector &mdash; a numeric constant used to refer to the
 * function in question. @name is the function name, as it is given in the
 * `glk.h` file, but without the “`glk_`” prefix.
 * And @fnptr is the address of the function itself.
 *
 * <note><para>
 *   This is included because it might be useful, but it is not recommended. To
 *   call an arbitrary Glk function, you should use gidispatch_call().
 * </para></note>
 *
 * See [Table of Selectors][chimara-Table-of-Selectors] for the selector
 * definitions.
 * See [Dispatching][chimara-Dispatching] for more about calling Glk functions
 * by selector.
 */
 
/**
 * gidispatch_get_function_by_id:
 * @id: A selector.
 *
 * Returns a structure describing the Glk function with selector @id. If there 
 * is no such function in the library, this returns %NULL.
 *
 * <note><para>
 *   Again, it is safest to assume that the structure is only valid until the
 *   next gidispatch_get_function() or gidispatch_get_function_by_id() call.
 * </para></note>
 *
 * Returns: a #gidispatch_function_t structure, or %NULL.
 */
 
/**
 * gidispatch_call:
 * @funcnum: Selector of the function to call.
 * @numargs: Length of @arglist.
 * @arglist: List of arguments to pass to the function.
 *
 * @funcnum is the function number to invoke; see [Table of
 * Selectors][chimara-Table-of-Selectors].
 * @arglist is the list of arguments, and @numargs is the length of the list.
 *
 * The arguments are all stored as #gluniversal_t objects.
 *
 * ## Basic Dispatch Types ## {#chimara-Basic-Dispatch-Types}
 *
 * Numeric arguments are passed in the obvious way &mdash; one argument per
 * #gluniversal_t, with the @uint or @sint field set to the numeric value.
 * Characters and strings are also passed in this way &mdash; #char<!---->s in
 * the @uch, @sch, or @ch fields (depending on whether the #char is signed) and
 * strings in the @charstr field. Opaque objects (windows, streams, etc) are
 * passed in the @opaqueref field (which is `void*`, in order to handle all
 * opaque pointer types.)
 *
 * However, pointers (other than C strings), arrays, and structures complicate
 * life.
 * So do return values.
 *
 * ## References ## {#chimara-References}
 *
 * A reference to a numeric type or object reference &mdash; that is,
 * [`glui32*`][glui32], [`winid_t*`][winid-t], and so on &mdash; takes
 * <emphasis>one or two</emphasis> #gluniversal_t objects. The first is a flag
 * indicating whether the reference argument is %NULL or not. The @ptrflag field
 * of this #gluniversal_t should be %FALSE (0) if the reference is %NULL, and
 * %TRUE (1) otherwise. If %FALSE, that is the end of the argument; you should
 * not use a #gluniversal_t to explicitly store the %NULL reference. If the flag
 * is %TRUE, you must then put a #gluniversal_t storing the base type of the
 * reference.
 *
 * For example, consider a hypothetical function, with selector `0xABCD`:
 * |[<!--language="C"-->
 * void glk_glomp(glui32 num, winid_t win, glui32 *numref, strid_t *strref);
 * ]|
 * ...and the calls:
 * |[<!--language="C"-->
 * glui32 value;
 * winid_t mainwin;
 * strid_t gamefile;
 * glk_glomp(5, mainwin, &value, &gamefile);
 * ]|
 *
 * To perform this through gidispatch_call(), you would do the following:
 * |[<!--language="C"-->
 * gluniversal_t arglist[6];
 * arglist[0].uint = 5;
 * arglist[1].opaqueref = mainwin;
 * arglist[2].ptrflag = TRUE;
 * arglist[3].uint = value;
 * arglist[4].ptrflag = TRUE;
 * arglist[5].opaqueref = gamefile;
 * gidispatch_call(0xABCD, 6, arglist);
 * value = arglist[3].uint;
 * gamefile = arglist[5].opaqueref;
 * ]|
 * 
 * Note that you copy the value of the reference arguments into and out of
 * @arglist.
 * Of course, it may be that `glk_glomp<!---->()` only uses these as pass-out
 * references or pass-in references; if so, you could skip copying in or out.
 *
 * For further examples:
 * |[<!--language="C"-->
 * glk_glomp(7, mainwin, NULL, NULL);
 * ...or...
 * gluniversal_t arglist[4];
 * arglist[0].uint = 7;
 * arglist[1].opaqueref = mainwin;
 * arglist[2].ptrflag = FALSE;
 * arglist[3].ptrflag = FALSE;
 * gidispatch_call(0xABCD, 4, arglist);
 * ]|
 *
 * |[<!--language="C"-->
 * glk_glomp(13, NULL, NULL, &gamefile);
 * ...or...
 * gluniversal_t arglist[5];
 * arglist[0].uint = 13;
 * arglist[1].opaqueref = NULL;
 * arglist[2].ptrflag = FALSE;
 * arglist[3].ptrflag = TRUE;
 * arglist[4].opaqueref = gamefile;
 * gidispatch_call(0xABCD, 5, arglist);
 * gamefile = arglist[4].opaqueref;
 * ]|
 *
 * |[<!--language="C"-->
 * glk_glomp(17, NULL, &value, NULL);
 * ...or...
 * gluniversal_t arglist[5];
 * arglist[0].uint = 17;
 * arglist[1].opaqueref = NULL;
 * arglist[2].ptrflag = TRUE;
 * arglist[3].uint = value;
 * arglist[4].ptrflag = FALSE;
 * gidispatch_call(0xABCD, 5, arglist);
 * value = arglist[3].uint;
 * ]|
 * 
 * As you see, the length of @arglist depends on how many of the reference
 * arguments are %NULL.
 *
 * ## Structures ## {#chimara-Structures}
 *
 * A structure pointer is represented by a single @ptrflag, possibly followed by
 * a sequence of #gluniversal_t objects (one for each field of the structure.)
 * Again, if the structure pointer is non-%NULL, the @ptrflag should be %TRUE
 * and be followed by values; if not, the @ptrflag should be %NULL and stands
 * alone.
 * 
 * For example, the function glk_select() can be invoked as follows:
 * |[<!--language="C"-->
 * event_t ev;
 * gluniversal_t arglist[5];
 * arglist[0].ptrflag = TRUE;
 * gidispatch_call(0x00C0, 5, arglist);
 * ev.type = arglist[1].uint;
 * ev.win = arglist[2].opaqueref;
 * ev.val1 = arglist[3].uint;
 * ev.val2 = arglist[4].uint;
 * ]|
 * 
 * Since the structure passed to glk_select() is a pass-out reference (the entry
 * values are ignored), you don't need to fill in `arglist[1..4]` before calling
 * gidispatch_call().
 * 
 * <note><para>
 *   Theoretically, you would invoke <code>#glk_select(%NULL)</code> by setting'
 *   <code>arglist[0].ptrflag</code> to %FALSE, and using a one-element @arglist
 *   instead of five-element. But it's illegal to pass %NULL to glk_select(). So
 *   you cannot actually do this.
 * </para></note>
 *
 * ## Arrays ## {#chimara-Arrays}
 *
 * In the Glk API, an array argument is always followed by a numeric argument
 * giving the array's length. These two C arguments are a single logical
 * argument, which is represented by <emphasis>one or three</emphasis>
 * #gluniversal_t objects. The first is a @ptrflag, indicating whether the
 * argument is %NULL or not. The second is a pointer, stored in the @array
 * field. The third is the array length, stored in the @uint field. And again,
 * if the @ptrflag is %NULL, the following two are omitted.
 * 
 * For example, the function glk_put_buffer() can be invoked as follows:
 * |[<!--language="C"-->
 * char buf[64];
 * glui32 len = 64;
 * glk_put_buffer(buf, len);
 * ...or...
 * gluniversal_t arglist[3];
 * arglist[0].ptrflag = TRUE;
 * arglist[1].array = buf;
 * arglist[2].uint = len;
 * gidispatch_call(0x0084, 3, arglist);
 * ]|
 * 
 * Since you are passing a C char array to gidispatch_call(), the contents will
 * be read directly from that. There is no need to copy data into @arglist, as
 * you would for a basic type.
 * 
 * If you are implementing a VM whose native representation of char arrays is
 * more complex, you will have to do more work. You should allocate a C char
 * array, copy your characters into it, make the call, and then free the array.
 *
 * <note><para>
 *   glk_put_buffer() does not modify the array passed to it, so there is no
 *   need to copy the characters out.
 * </para></note>
 *
 * ## Return Values ## {#chimara-Return-Values}
 *
 * The return value of a function is not treated specially. It is simply
 * considered to be a pass-out reference argument which may not be %NULL. It
 * comes after all the other arguments of the function.
 * 
 * For example, the function glk_window_get_rock() can be invoked as follows:
 * |[<!--language="C"-->
 * glui32 rock;
 * winid_t win;
 * rock = glk_window_get_rock(win);
 * ...or...
 * gluniversal_t arglist[3];
 * arglist[0].opaqueref = win;
 * arglist[1].ptrflag = TRUE;
 * gidispatch_call(0x0021, 3, arglist);
 * rock = arglist[2].uint;
 * ]|
 */

/**
 * gluniversal_t:
 * @uint: Stores a #glui32.
 * @sint: Stores a #glsi32.
 * @opaqueref: Stores a #winid_t, #strid_t, #frefid_t, or #schanid_t.
 * @uch: Stores an #unsigned #char.
 * @sch: Stores a #signed #char.
 * @ch: Stores a #char with the default signedness.
 * @charstr: Stores a null-terminated string.
 * @unicharstr: Stores a zero-terminated string of #glui32 values representing
 * Unicode characters.
 * @array: Stores a pointer to an array, and should be followed by another 
 * #gluniversal_t with the array length stored in the @uint member.
 * @ptrflag: If %FALSE, represents an opaque reference or array that is %NULL,
 * in which case it represents the entire argument. If %TRUE, should be followed
 * by another #gluniversal_t with the pointer in its @opaqueref or @array field.
 *
 * This is a union, encompassing all the types that can be passed to Glk
 * functions.
 */
 
/**
 * gidispatch_prototype:
 * @funcnum: A selector for the function to be queried.
 *
 * This returns a string which encodes the proper argument list for the given
 * function. If there is no such function in the library, this returns %NULL.
 * 
 * The prototype string for the `glk_glomp<!---->()` function described above would be:
 * `"4IuQa&amp;Iu&amp;Qb:"`.
 * The `"4"` is the number of arguments (including the return value, if there is
 * one, which in this case there isn't.)
 * `"Iu"` denotes an unsigned integer; `"Qa"` is an opaque object of class 0
 * (window).
 * `"&amp;Iu"` is a <emphasis>reference</emphasis> to an unsigned integer, and
 * `"&amp;Qb"` is a reference to a stream.
 * The colon at the end terminates the argument list; the return value would
 * follow it, if there was one.
 *
 * Note that the initial number (`"4"` in this case) is the number of logical
 * arguments, not the number of #gluniversal_t objects which will be passed to
 * gidispatch_call().
 * The `glk_glomp<!---->()` call uses anywhere from four to six #gluniversal_t objects,
 * as demonstrated above.
 *
 * The basic type codes:
 * <variablelist>
 * <varlistentry>
 *   <term><code>Iu, Is</code></term>
 *   <listitem><para>Unsigned and signed 32-bit integer.</para></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term><code>Cn, Cu, Cs</code></term>
 *   <listitem><para>Character, #unsigned #char, and #signed #char.</para>
 *     <note><para>Of course <code>Cn</code> will be the same as either 
 *     <code>Cu</code> or <code>Cs</code>, depending on the platform. For this
 *     reason, Glk avoids using it, but it is included here for completeness.
 *     </para></note>
 *   </listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term><code>S</code></term>
 *   <listitem><para>A C-style string (null-terminated array of #char). In Glk,
 *   strings are always treated as read-only and used immediately; the library
 *   does not retain a reference to a string between Glk calls. A Glk call that
 *   wants to use writable char arrays will use an array type 
 *   (<code>"&num;C"</code>), not string (<code>"S"</code>).</para></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term><code>U</code></term>
 *   <listitem><para>A zero-terminated array of 32-bit integers. This is
 *   primarily intended as a Unicode equivalent of <code>"S"</code>. Like 
 *   <code>"S"</code> strings, <code>"U"</code> strings are read-only and used
 *   immediately. A Glk call that wants to use writable Unicode arrays will use
 *   an array type (<code>"&num;Iu"</code>) instead of <code>"U"</code>.</para>
 *   </listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term><code>F</code></term>
 *   <listitem><para>A floating-point value. Glk does not currently use
 *   floating-point values, but we might as well define a code for them.</para>
 *   </listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term><code>Qa, Qb, Qc...</code></term>
 *   <listitem><para>A reference to an opaque object. The second letter
 *   determines which class is involved. (The number of classes can be gleaned
 *   from gidispatch_count_classes(); see <link 
 *   linkend="chimara-Interrogating-the-Interface">Interrogating the
 *   Interface</link>).</para>
 *   <note><para>
 *     If Glk expands to have more than 26 classes, we'll think of something.
 *   </para></note></listitem>
 * </varlistentry>
 * </variablelist>
 * Any type code can be prefixed with one or more of the following characters:
 * <variablelist>
 * <varlistentry>
 *   <term><code>&amp;</code></term>
 *   <listitem><para>A reference to the type; or, if you like, a variable passed
 *   by reference. The reference is passed both in and out, so you must copy the
 *   value in before calling gidispatch_call() and copy it out afterward.</para>
 *   </listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term><code>&lt;</code></term>
 *   <listitem><para>A reference which is pass-out only. The initial value is
 *   ignored, so you only need copy out the value after the call.</para>
 *   </listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term><code>&gt;</code></term>
 *   <listitem><para>A reference which is pass-in only.</para>
 *   <note><para>
 *     This is not generally used for simple types, but is useful for structures
 *     and arrays.
 *   </para></note></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term><code>+</code></term>
 *   <listitem><para>Combined with <code>"&"</code>, <code>"&lt;"</code>, or 
 *   <code>"&gt;"</code>, indicates that a valid reference is mandatory; %NULL
 *   cannot be passed.</para>
 *   <note><para>
 *     Note that even though the @ptrflag #gluniversal_t for a <code>"+"</code>
 *     reference is always %TRUE, it cannot be omitted.
 *   </para></note></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term><code>:</code></term>
 *   <listitem><para>The colon separates the arguments from the return value, or
 *   terminates the string if there is no return value. Since return values are
 *   always non-%NULL pass-out references, you may treat <code>":"</code> as
 *   equivalent to <code>"&lt;+"</code>. The colon is never combined with any
 *   other prefix character.</para></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term><code>[...]</code></term>
 *   <listitem><para>Combined with <code>"&amp;"</code>, <code>"&lt;"</code>, or 
 *   <code>"&gt;"</code>, indicates a structure reference. Between the brackets
 *   is a complete argument list encoding string, including the number of
 *   arguments.</para>
 *   <note><para>
 *     For example, the prototype string for glk_select() is
 *     <code>"1&lt;+[4IuQaIuIu]:"</code> &mdash; one argument, which is a
 *     pass-out non-%NULL reference to a structure, which contains four
 *     arguments.
 *   </para></note>
 *   <para>Currently, structures in Glk contain only basic types.</para>
 *   </listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term><code>&num;</code></term>
 *   <listitem><para>Combined with <code>"&amp;"</code>, <code>"&lt;"</code>, or 
 *   <code>"&gt;"</code>, indicates an array reference. As described above, this
 *   encompasses up to three #gluniversal_t objects &mdash; @ptrflag, pointer,
 *   and integer length.</para>
 *   <note><para>
 *     Depending on the design of your program, you may wish to pass a pointer
 *     directly to your program's memory, or allocate an array and copy the
 *     contents in and out.
 *     See [Arrays][chimara-Arrays].
 *   </para></note></listitem>
 * </varlistentry>
 * <varlistentry>
 *   <term><code>!</code></term>
 *   <listitem><para>Combined with <code>"&num;"</code>, indicates that the
 *   array is retained by the library. The library will keep a reference to the
 *   array; the contents are undefined until further notice. You should not use
 *   or copy the contents of the array out after the call, even for 
 *   <code>"&amp;&num;!"</code> or <code>"&lt;&num;!"</code> arrays. Instead, do
 *   it when the library releases the array.</para>
 *   <note><para>
 *     For example, glk_stream_open_memory() retains the array that you pass it,
 *     and releases it when the stream is closed. The library can notify you
 *     automatically when arrays are retained and released; see [Retained Array
 *     Registry][gidispatch-set-retained-registry].
 *   </para></note></listitem>
 * </varlistentry>
 * </variablelist>
 *
 * The order of these characters and prefixes is not completely arbitrary. Here
 * is a formal grammar for the prototype strings.
 *
 * <note><para>Thanks to Neil Cerutti for working this out.</para></note>
 *
 * <productionset>
 * <production id="prototype">
 *   <lhs>prototype</lhs>
 *   <rhs>ArgCount [ <nonterminal def="&num;arg_list">arg_list</nonterminal> ]
 *     ':' [ <nonterminal def="&num;arg">arg</nonterminal> ] EOL</rhs>
 * </production>
 * <production id="arg_list">
 *   <lhs>arg_list</lhs>
 *   <rhs><nonterminal def="&num;arg">arg</nonterminal> { <nonterminal
 *     def="&num;arg">arg</nonterminal> }</rhs>
 * </production>
 * <production id="arg">
 *   <lhs>arg</lhs>
 *   <rhs>TypeName | <nonterminal def="&num;ref_type">ref_type</nonterminal>
 *   </rhs>
 * </production>
 * <production id="ref_type">
 *   <lhs>ref_type</lhs>
 *   <rhs>RefType [ '+' ] <nonterminal
 *     def="&num;target_type">target_type</nonterminal></rhs>
 * </production>
 * <production id="target_type">
 *   <lhs>target_type</lhs>
 *   <rhs>TypeName | <nonterminal def="&num;array">array</nonterminal> |
 *     <nonterminal def="&num;struct">struct</nonterminal></rhs>
 * </production>
 * <production id="array">
 *   <lhs>array</lhs>
 *   <rhs>'&num;' [ '!' ] TypeName</rhs>
 * </production>
 * <production id="struct">
 *   <lhs>struct</lhs>
 *   <rhs>'[' ArgCount [ <nonterminal def="&num;arg_list">arg_list</nonterminal>
 *     ] ']'</rhs>
 * </production>
 * </productionset>
 * <constraintdef id="TypeName">
 *   <para>TypeName is <code>I[us]<!---->|C[nus]<!---->|S|U|F|Q[a-z]</code>
 *   </para>
 * </constraintdef>
 * <constraintdef id="ArgCount">
 *   <para>ArgCount is <code>\d+</code></para>
 * </constraintdef>
 * <constraintdef id="RefType">
 *   <para>RefType is <code>&amp;|&lt;|&gt;</code></para>
 * </constraintdef>
 * <constraintdef id="EOL">
 *   <para>EOL is end of input</para>
 * </constraintdef>
 *
 * Returns: A string which encodes the prototype of the specified Glk function.
 */

/**
 * gidisp_Class_Window:
 *
 * Represents a #winid_t opaque object.
 */
 
/**
 * gidisp_Class_Stream:
 *
 * Represents a #strid_t opaque object.
 */
 
/**
 * gidisp_Class_Fileref:
 *
 * Represents a #frefid_t opaque object.
 */

/**
 * gidisp_Class_Schannel:
 * 
 * Represents a #schanid_t opaque object.
 */

/**
 * gidispatch_rock_t:
 * @num: Space for storing an integer.
 * @ptr: Space for storing a pointer.
 *
 * You can store any value you want in this object; return it from your object
 * registry and retained array registry callbacks, and the library will stash it
 * away. You can retrieve it with gidispatch_get_objrock().
 */

/**
 * GI_DISPA_GAME_ID_AVAILABLE:
 *
 * The game should test `#ifdef GI_DISPA_GAME_ID_AVAILABLE` to ensure that these
 * functions exist.
 * (They are a late addition to <filename>gi_dispa.c</filename>, so older Glk
 * library distributions will lack them.)
 *
 * Stability: Unstable
 */

/**
 * gidispatch_set_game_id_hook:
 * @hook: a function that returns a game ID string.
 *
 * Set a function for getting a game ID string.
 * The Glk library may call the supplied function when creating files, so that
 * the files can be put in a game-specific location.
 *
 * The function must have the form: `char *func(void);`
 *
 * It should return %NULL or a pointer to a (null-terminated) string.
 * (The string will be copied, so it may be in a temporary buffer.)
 *
 * Stability: Unstable
 */

/**
 * gidispatch_get_game_id:
 *
 * Retrieve a game ID string for the current game.
 *
 * If not %NULL, this string may be in a temporary buffer, so the caller
 * must copy it!
 *
 * Returns: (nullable): A game ID string.
 *
 * Stability: Unstable
 */

/**
 * GIDISPATCH_AUTORESTORE_REGISTRY:
 *
 * Stability: Unstable
 */

/*---------- TYPES, FUNCTIONS AND CONSTANTS FROM GI_BLORB.H ------------------*/
 
/**
 * giblorb_err_t: 
 *
 * An integer type that can hold the Blorb error codes.
 */ 
 
/**
 * giblorb_err_None:
 *
 * No error.
 */
 
/**
 * giblorb_err_CompileTime: 
 *
 * Something is compiled wrong in the Blorb layer.
 */
 
/**
 * giblorb_err_Alloc: 
 *
 * Memory could not be allocated.
 *
 * > # Chimara #
 * > The Blorb layer in the Chimara library should not return this error code;
 * > instead, the program aborts if memory allocation fails, in keeping with
 * > GLib practices.
 */
 
/**
 * giblorb_err_Read: 
 *
 * Data could not be read from the file.
 */

/** 
 * giblorb_err_NotAMap:
 *
 * The map parameter is invalid.
 */

/** 
 * giblorb_err_Format:
 *
 * The Blorb file is corrupted or invalid.
 */
 
/**
 * giblorb_err_NotFound:
 *
 * The requested data could not be found.
 */

/**
 * giblorb_method_DontLoad:
 *
 * Pass this to giblorb_load_chunk_by_type(), giblorb_load_chunk_by_number(), or
 * giblorb_load_resource() to obtain information about a chunk without actually
 * loading it.
 */

/**
 * giblorb_method_Memory:
 *
 * Pass this to giblorb_load_chunk_by_type(), giblorb_load_chunk_by_number(), or
 * giblorb_load_resource() to load a chunk into memory.
 */

/**
 * giblorb_method_FilePos:
 *
 * Pass this to giblorb_load_chunk_by_type(), giblorb_load_chunk_by_number(), or
 * giblorb_load_resource() to get the position in the Blorb file at which the
 * chunk data starts.
 */

/**
 * giblorb_ID_Snd:
 *
 * Resource usage constant representing a sound file.
 */

/**
 * giblorb_ID_Exec:
 *
 * Resource usage constant representing an executable program.
 */
 
/**
 * giblorb_ID_Pict:
 *
 * Resource usage constant representing an image file.
 */

/**
 * giblorb_ID_Data:
 *
 * Resource usage constant representing a data file.
 */

/**
 * giblorb_ID_Copyright:
 *
 * Chunk type constant representing the copyright message (date and holder,
 * without the actual copyright symbol). There should only be one such chunk per
 * file.
 */

/**
 * giblorb_ID_AUTH:
 *
 * Chunk type constant representing the name of the author or creator of the
 * file. This could be a login name on multi-user systems, for example. There
 * should only be one such chunk per file.
 */

/**
 * giblorb_ID_ANNO:
 *
 * Chunk type constant representing any textual annotation that the user or
 * writing program sees fit to include.
 */

/**
 * giblorb_ID_TEXT:
 *
 * Chunk type constant representing a text data file.
 */

/**
 * giblorb_ID_BINA:
 *
 * Chunk type constant representing a binary data file.
 */

/**
 * giblorb_ID_PNG:
 *
 * Chunk type constant representing a PNG image file.
 *
 * Stability: Unstable
 */

/**
 * giblorb_ID_JPEG:
 *
 * Chunk type constant representing a JPEG image file.
 *
 * Stability: Unstable
 */

/**
 * giblorb_map_t:
 *
 * Holds the complete description of an open Blorb file. This type is opaque for
 * normal interpreter use.
 */
 
/**
 * giblorb_result_t:
 * @chunknum: The chunk number (for use in giblorb_unload_chunk(), etc.)
 * @length: The length of the data
 * @chunktype: The type of the chunk.
 *
 * Holds information about a chunk loaded from a Blorb file, and the method of
 * accessing the chunk data. @data is a union of @ptr, a pointer to the data (if
 * you used %giblorb_method_Memory) and @startpos, the position in the file (if
 * you used %giblorb_method_FilePos). See giblorb_load_chunk_by_type() and
 * giblorb_load_chunk_by_number(). 
 */

/**
 * giblorb_image_info_t:
 * @chunktype: The type of the chunk (%giblorb_ID_PNG or %giblorb_ID_JPEG)
 * @width:
 * @height:
 * @alttext:
 *
 * Stability: Unstable
 */

/**
 * giblorb_create_map:
 * @file: An input stream pointing to a Blorb file.
 * @newmap: Return location for a Blorb resource map.
 *
 * Reads Blorb data out of a Glk stream. It does not load every resource at 
 * once; instead, it creates a map in memory which makes it easy to find 
 * resources. A pointer to the map is stored in @newmap. This is an opaque 
 * object; you pass it to the other Blorb-layer functions.
 *
 * Returns: a Blorb error code. 
 */
 
/**
 * giblorb_destroy_map: 
 * @map: A Blorb resource map to deallocate.
 *
 * Deallocates @map and all associated memory. This does 
 * <emphasis>not</emphasis> close the original stream.
 *
 * Returns: a Blorb error code. 
 */

/**
 * giblorb_load_chunk_by_type:
 * @map: The Blorb resource map to load a chunk from.
 * @method: The loading method to use, one of %giblorb_method_DontLoad, 
 * %giblorb_method_Memory, or %giblorb_method_FilePos.
 * @res: Return location for the result.
 * @chunktype: The type of chunk to load.
 * @count: The chunk number of type @chunktype to load.
 *
 * Loads a chunk of a given type. The @count parameter distinguishes between 
 * chunks of the same type. If @count is zero, the first chunk of that type is 
 * loaded, and so on.
 * 
 * To load a chunk of an IFF FORM type (such as AIFF), you should pass in the 
 * form type, rather than FORM.
 * <note><para>
 *  This introduces a slight ambiguity &mdash; you cannot distiguish between a 
 *  FORM AIFF chunk and a non-FORM chunk of type AIFF. However, the latter is 
 *  almost certainly a mistake.
 * </para></note> 
 * 
 * The returned data is written into @res, according to @method.
 * 
 * The <structfield>chunknum</structfield> field is filled in with the number of
 * the chunk. (This value can then be passed to giblorb_load_chunk_by_number() 
 * or giblorb_unload_chunk().) The <structfield>length</structfield> field is 
 * filled in with the length of the chunk in bytes. The 
 * <structfield>chunktype</structfield> field is the chunk's type, which of 
 * course will be the type you asked for.
 * 
 * If you specify %giblorb_method_DontLoad, no data is actually loaded in. You
 * can use this if you are only interested in whether a chunk exists, or in the
 * <structfield>chunknum</structfield> and <structfield>length</structfield> 
 * parameters.
 * 
 * If you specify %giblorb_method_FilePos, 
 * <structfield>data.startpos</structfield> is filled in with the file position
 * of the chunk data. You can use glk_stream_set_position() to read the data 
 * from the stream.
 * 
 * If you specify %giblorb_method_Memory, <structfield>data.ptr</structfield> is
 * filled with a pointer to allocated memory containing the chunk data. This 
 * memory is owned by the map, not you. If you load the chunk more than once 
 * with %giblorb_method_Memory, the Blorb layer is smart enough to keep just one
 * copy in memory. You should not deallocate this memory yourself; call 
 * giblorb_unload_chunk() instead.
 *
 * Returns: a Blorb error code.
 */

/** 
 * giblorb_load_chunk_by_number:
 * @map: The Blorb resource map to load a chunk from.
 * @method: The loading method to use, one of %giblorb_method_DontLoad, 
 * %giblorb_method_Memory, or %giblorb_method_FilePos.
 * @res: Return location for the result.
 * @chunknum: The chunk number to load.
 *
 * This is similar to giblorb_load_chunk_by_type(), but it loads a chunk with a
 * given chunk number. The type of the chunk can be found in the 
 * <structfield>chunktype</structfield> field of #giblorb_result_t. You can get
 * the chunk number from the <structfield>chunknum</structfield> field, after 
 * calling one of the other load functions.
 *
 * Returns: a Blorb error code. 
 */

/**
 * giblorb_unload_chunk:
 * @map: The Blorb resource map to unload a chunk from.
 * @chunknum: The chunk number to unload.
 *
 * Frees the chunk data allocated by %giblorb_method_Memory. If the given chunk
 * has never been loaded into memory, this has no effect. 
 *
 * Returns: a Blorb error code.
 */

/**
 * giblorb_load_resource:
 * @map: The Blorb resource map to load a resource from.
 * @method: The loading method to use, one of %giblorb_method_DontLoad, 
 * %giblorb_method_Memory, or %giblorb_method_FilePos.
 * @res: Return location for the result.
 * @usage: The type of data resource to load.
 * @resnum: The resource number to load.
 *
 * Loads a resource, given its usage and resource number. Currently, the three
 * usage values are %giblorb_ID_Pict (images), %giblorb_ID_Snd (sounds), and
 * %giblorb_ID_Exec (executable program). See the Blorb specification for more
 * information about the types of data that can be stored for these usages.
 * 
 * Note that a resource number is not the same as a chunk number. The resource
 * number is the sound or image number specified by a Glk program. Chunk number
 * is arbitrary, since chunks in a Blorb file can be in any order. To find the
 * chunk number of a given resource, call giblorb_load_resource() and look in
 * <structfield>res.chunknum</structfield>.
 *
 * Returns: a Blorb error code.
 */

/**
 * giblorb_count_resources:
 * @map: The Blorb resource map in which to count the resources.
 * @usage: The type of data resource to count.
 * @num: Return location for the number of chunks of @usage.
 * @min: Return location for the lowest resource number of @usage.
 * @max: Return location for the highest resource number of @usage.
 *
 * Counts the number of chunks with a given usage (image, sound, or executable.)
 * The total number of chunks of that usage is stored in @num. The lowest and 
 * highest resource number of that usage are stored in @min and @max. You can
 * leave any of the three pointers %NULL if you don't care about that
 * information. 
 *
 * Returns: a Blorb error code.
 */

/**
 * giblorb_load_image_info:
 * @map: The Blorb resource map from which to load the image info.
 * @resnum: The resource number to examine.
 * @res: Return location for the result.
 *
 * Returns: a Blorb error code.
 *
 * Stability: Unstable
 */

/*--------------------TYPES AND CONSTANTS FROM GLKSTART.H---------------------*/

/**
 * glkunix_argumentlist_t:
 * @name: the option as it would appear on the command line (including the 
 * leading dash, if any.) 
 * @desc: a description of the argument; this is used when the library is 
 * printing a list of options.
 * @argtype: one of the `glkunix_arg_` constants.
 * 
 * <variablelist>
 * <varlistentry>
 *  <term>%glkunix_arg_NoValue</term>
 *  <listitem><para>The argument appears by itself.</para></listitem>
 * </varlistentry>
 * <varlistentry>
 *  <term>%glkunix_arg_ValueFollows</term>
 *  <listitem><para>The argument must be followed by another argument (the 
 *  value).</para></listitem>
 * </varlistentry>
 * <varlistentry>
 *  <term>%glkunix_arg_ValueCanFollow</term> 
 *  <listitem><para>The argument may be followed by a value, optionally. (If the
 *  next argument starts with a dash, it is taken to be a new argument, not the 
 *  value of this one.)</para></listitem>
 * </varlistentry>
 * <varlistentry>
 *  <term>%glkunix_arg_NumberValue</term>
 *  <listitem><para>The argument must be followed by a number, which may be the 
 *  next argument or part of this one. (That is, either “<code>-width 20</code>”
 *  or “<code>-width20</code>” will be accepted.)
 *  </para></listitem>
 * </varlistentry>
 * <varlistentry> 
 *  <term>%glkunix_arg_End</term>
 *  <listitem><para>The <code>glkunix_arguments[]</code> array must be 
 *  terminated with an entry containing this value.</para></listitem>
 * </varlistentry>
 * </variablelist>
 * 
 * To accept arbitrary arguments which lack dashes, specify a name of `""` and
 * an argtype of %glkunix_arg_ValueFollows.
 *
 * If you don't care about command-line arguments, you must still define an
 * empty arguments list, as follows:
 * |[<!--language="C"-->
 * glkunix_argumentlist_t glkunix_arguments[] = {
 *     { NULL, glkunix_arg_End, NULL }
 * };
 * ]|
 * 
 * Here is a more complete sample list:
 * |[<!--language="C"-->
 * glkunix_argumentlist_t glkunix_arguments[] = {
 *     { "", glkunix_arg_ValueFollows, "filename: The game file to load." },
 *     { "-hum", glkunix_arg_ValueFollows, "-hum NUM: Hum some NUM." },
 *     { "-bom", glkunix_arg_ValueCanFollow, "-bom [ NUM ]: Do a bom (on
 *       the NUM, if given)." },
 *     { "-goo", glkunix_arg_NoValue, "-goo: Find goo." },
 *     { "-wob", glkunix_arg_NumberValue, "-wob NUM: Wob NUM times." },
 *     { NULL, glkunix_arg_End, NULL }
 * };
 * ]|
 * This would match the arguments “`thingfile -goo -wob8 -bom -hum song`”.
 *
 * After the library parses the command line, it does various occult rituals of
 * initialization, and then calls glkunix_startup_code().
 *
 * |[<!--language="C"-->
 * int glkunix_startup_code(glkunix_startup_t *data);
 * ]|
 *
 * This should return %TRUE if everything initializes properly. If it returns
 * %FALSE, the library will shut down without ever calling your glk_main() 
 * function.
 */

/**
 * glkunix_startup_t: 
 * @argc: The number of arguments in @argv.
 * @argv: Strings representing command line arguments.
 * 
 * The fields are a standard Unix `(argc, argv)` list, which contain the
 * arguments you requested from the command line.
 * In deference to custom, `argv[0]` is always the program name.
 */

/**
 * glkunix_arg_End:
 *
 * Terminates a list of #glkunix_argumentlist_t.
 */
 
/**
 * glkunix_arg_ValueFollows:
 *
 * Indicates an argument which must be followed by a value, as the next 
 * argument.
 */

/** 
 * glkunix_arg_NoValue:
 *
 * Indicates an argument which occurs by itself, without a value.
 */
 
/**
 * glkunix_arg_ValueCanFollow:
 *
 * Indicates an argument which may be followed by a value, or may occur by 
 * itself.
 */
 
/**
 * glkunix_arg_NumberValue:
 *
 * Indicates an argument which must be followed by a numerical value, either as 
 * the next argument or tacked onto the end of this argument.
 */
