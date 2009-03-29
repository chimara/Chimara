/*
 * doc.c - Contains the short and long descriptions of all the documentation
 *         sections in the Glk spec, as well as the GtkDoc comments for symbols
 *         defined only in glk.h.
 */

/**
 * SECTION:glk-exiting
 * @short_description: How to terminate a Glk program cleanly
 * @include: glk.h
 *
 * A Glk program usually ends when the end of the glk_main() function is 
 * reached. You can also terminate it earlier.
 */ 

/**
 * SECTION:glk-interrupt
 * @short_description: Specifying an interrupt handler for cleaning up critical
 * resources
 * @include: glk.h
 *
 * Most platforms have some provision for interrupting a program &mdash;
 * <keycombo action="simul"><keycap function="command">command</keycap>
 * <keycap>period</keycap></keycombo> on the Macintosh, <keycombo 
 * action="simul"><keycap function="control">control</keycap><keycap>C</keycap>
 * </keycombo> in Unix, possibly a window manager item, or other possibilities.
 * This can happen at any time, including while execution is nested inside one 
 * of your own functions, or inside a Glk library function.
 *
 * If you need to clean up critical resources, you can specify an interrupt
 * handler function.
 */

/**
 * SECTION:glk-tick
 * @short_description: Yielding time to the operating system
 * @include: glk.h
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
 * @include: glk.h
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
 * @include: glk.h
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
 * the allocation function will return %NULL (0) instead of a valid pointer. You
 * should always test for this possibility.
 * 
 * %NULL is never the identifier of any object (window, stream, file reference,
 * or sound channel). The value %NULL is often used to indicate <quote>no
 * object</quote> or <quote>nothing</quote>, but it is not a valid reference. If
 * a Glk function takes an object reference as an argument, it is illegal to
 * pass in %NULL unless the function definition says otherwise.
 * 
 * The <filename class="headerfile">glk.h</filename> file defines types
 * #winid_t, #strid_t, #frefid_t, #schanid_t to store references. These are
 * pointers to struct #glk_window_struct, #glk_stream_struct, 
 * #glk_fileref_struct, and #glk_schannel_struct respectively. It is, of course,
 * illegal to pass one kind of pointer to a function which expects another.
 * 
 * <note><para>
 * This is how you deal with opaque objects from a C program. If you are using
 * Glk through a virtual machine, matters will probably be different. Opaque
 * objects may be represented as integers, or as VM objects of some sort.
 * </para></note>
 * <refsect2 id="chimara-Rocks"><!-- Indeed it does. -->
 * <title>Rocks</title>
 * <para>
 * Every one of these objects (window, stream, file reference, or sound channel)
 * has a <quote>rock</quote> value. This is simply a 32-bit integer value which
 * you provide, for your own purposes, when you create the object.
 * </para>
 * <note><para>The library &mdash; so to speak &mdash; stuffs this value under a
 * rock for safe-keeping, and gives it back to you when you ask for it.
 * </para></note>
 * <note><para>If you don't know what to use the rocks for, provide 0 and forget
 * about it.</para></note>
 * </refsect2>
 * <refsect2 id="chimara-Iterating-Through-Opaque-Objects">
 * <title>Iteration Through Opaque Objects</title>
 * <para>
 * For each class of opaque objects, there is an iterate function, which you can
 * use to obtain a list of all existing objects of that class. It takes the form
 * <informalexample><programlisting>
 * <replaceable>CLASS</replaceable>id_t glk_<replaceable>CLASS</replaceable>_iterate(<replaceable>CLASS</replaceable>id_t <parameter>obj</parameter>, #glui32 *<parameter>rockptr</parameter>);
 * </programlisting></informalexample>
 * ...where <code><replaceable>CLASS</replaceable></code> represents one of the
 * opaque object classes. 
 * </para>
 * <note><para>
 *   So, at the current time, these are the functions glk_window_iterate(),
 *   glk_stream_iterate(), glk_fileref_iterate(), and glk_schannel_iterate().  
 *   There may be more classes in future versions of the spec; they all behave
 *   the same.
 * </para></note>
 * <para>
 * Calling <code>glk_<replaceable>CLASS</replaceable>_iterate(%NULL, r)</code>
 * returns the first object; calling 
 * <code>glk_<replaceable>CLASS</replaceable>_iterate(obj, r)</code> returns
 * the next object, until there aren't any more, at which time it returns %NULL.
 * </para>
 * <para>
 * The @rockptr argument is a pointer to a location; whenever  
 * <code>glk_<replaceable>CLASS</replaceable>_iterate()</code> returns an
 * object, the object's rock is stored in the location <code>(*@rockptr)</code>.
 * If you don't want the rocks to be returned, you may set @rockptr to %NULL.
 * </para>
 * <para>
 * You usually use this as follows:
 * <informalexample><programlisting>
 * obj = glk_<replaceable>CLASS</replaceable>_iterate(NULL, NULL);
 * while (obj) {
 *    /* ...do something with obj... *<!-- -->/
 *    obj = glk_<replaceable>CLASS</replaceable>_iterate(obj, NULL);
 * }
 * </programlisting></informalexample>
 * </para>
 * <para>
 * If you create or destroy objects inside this loop, obviously, the results are
 * unpredictable. However it is always legal to call 
 * <code>glk_<replaceable>CLASS</replaceable>_iterate(obj, r)</code> as long as
 * @obj is a valid object id, or %NULL.
 * </para>
 * <para>
 * The order in which objects are returned is entirely arbitrary. The library
 * may even rearrange the order every time you create or destroy an object of
 * the given class. As long as you do not create or destroy any object, the rule
 * is that <code>glk_<replaceable>CLASS</replaceable>_iterate(obj, r)</code> has
 * a fixed result, and iterating through the results as above will list every
 * object exactly once. 
 * </para>
 * </refsect2>
 */

/**
 * SECTION:glk-gestalt
 * @short_description: Testing Glk's capabilities
 * @include: glk.h
 *
 * The <quote>gestalt</quote> mechanism (cheerfully stolen from the Mac OS) is a
 * system by which the Glk API can be upgraded without making your life
 * impossible. New capabilities (graphics, sound, or so on) can be added without
 * changing the basic specification. The system also allows for 
 * <quote>optional</quote> capabilities &mdash; those which not all Glk library
 * implementations will support &mdash; and allows you to check for their
 * presence without trying to infer them from a version number.
 * 
 * The basic idea is that you can request information about the capabilities of
 * the API, by calling the gestalt functions.
 */

/**
 * SECTION:glk-character-input
 * @short_description: Waiting for a single keystroke
 * @include: glk.h
 *
 * You can request that the player hit a single key. See <link 
 * linkend="chimara-Character-Input-Events">Character Input Events</link>.
 * 
 * If you use the basic text API, the character code which is returned can be
 * any value from 0 to 255. The printable character codes have already been
 * described. The remaining codes are typically control codes: <keycombo  
 * action="simul"><keycap function="control">control</keycap>
 * <keycap>A</keycap></keycombo> to <keycombo action="simul"><keycap 
 * function="control">control</keycap><keycap>Z</keycap></keycombo> and a few
 * others.
 * 
 * There are also a number of special codes, representing special keyboard
 * keys, which can be returned from a char-input event. These are represented
 * as 32-bit integers, starting with 4294967295 (0xFFFFFFFF) and working down.
 * The special key codes are defined in the <filename 
 * class="headerfile">glk.h</filename> file. They include one code for <keycap
 * function="enter">return</keycap> or <keycap function="enter">enter</keycap>,
 * one for <keycap function="delete">delete</keycap> or <keycap
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
 * <keysym>#keycode_Tab</keysym> event (value 0xFFFFFFF7) when this occurs.
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
 *   <quote><computeroutput>press the <keycap function="tab">tab</keycap>
 *   key</computeroutput></quote>, you should check for a 
 *   <keysym>#keycode_Tab</keysym> event as opposed to a <keycombo 
 *   action="simul"><keycap function="control">control</keycap>
 *   <keycap>I</keycap></keycombo> event.
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
 * purposes of the interface. For example, the Mac Glk library reserves the 
 * <keycap function="tab">tab</keycap> key for switching between different Glk
 * windows. Therefore, on the Mac, the library will never generate a
 * <keysym>#keycode_Tab</keysym> event or a <keycombo action="simul">
 * <keycap function="control">control</keycap><keycap>I</keycap></keycombo>
 * event.
 * 
 * <note><para>
 *   Note that the linefeed or <keycombo action="simul"><keycap  
 *   function="control">control</keycap><keycap>J</keycap></keycombo> 
 *   character, which is the only printable control character, is probably not
 *   typable. This is because, in most libraries, it will be converted to
 *   <keysym>#keycode_Return</keysym>. Again, you should check for
 *   <keysym>#keycode_Return</keysym> if your program asks the player to 
 *   <quote><computeroutput>press the <keycap function="enter">return</keycap>
 *   key</computeroutput></quote>.
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
 * You can test for this by using the #gestalt_CharInput selector.
 * 
 * <note><para>
 *   Glk porters take note: it is not a goal to be able to generate every
 *   single possible key event. If the library says that it can generate a
 *   particular keycode, then game programmers will assume that it is
 *   available, and ask players to use it. If a <keysym>#keycode_Home</keysym>
 *   event can only be generated by typing <keycombo action="seq"><keycap
 *   function="escape">escape</keycap><keycombo action="simul"><keycap
 *   function="control">control</keycap><keycap>A</keycap></keycombo>
 *   </keycombo>, and the player does not know this, the player will be lost
 *   when the game says <quote><computeroutput>Press the <keycap
 *   function="home">home</keycap> key to see the next 
 *   hint.</computeroutput></quote> It is better for the library to say that it
 *   cannot generate a <keysym>#keycode_Home</keysym> event; that way the game
 *   can detect the situation and ask the user to type <keycap>H</keycap>
 *   instead.
 * </para>
 * <para>
 *   Of course, it is better not to rely on obscure keys in any case. The arrow
 *   keys and <keycap function="enter">return</keycap> are nearly certain to be
 *   available; the others are of gradually decreasing reliability, and you
 *   (the game programmer) should not depend on them. You must be certain to
 *   check for the ones you want to use, including the arrow keys and <keycap
 *   function="enter">return</keycap>, and be prepared to use different keys in
 *   your interface if #gestalt_CharInput says they are not available.
 * </para></note>
 */

/**
 * SECTION:glk-case
 * @short_description: Changing the case of strings
 * @include: glk.h
 *
 * Glk has functions to manipulate the case of both Latin-1 and Unicode strings.
 * One Latin-1 lowercase character corresponds to one uppercase character, and
 * vice versa, so the Latin-1 functions act on single characters. The Unicode
 * functions act on whole strings, since the length of the string may change.
 */

/**
 * SECTION:glk-window-opening
 * @short_description: Creating new windows and closing them
 * @include: glk.h
 *
 * You can open a new window using glk_window_open() and close it again using
 * glk_window_close().
 */

/**
 * SECTION:glk-window-constraints
 * @short_description: Manipulating the size of a window
 * @include: glk.h
 *
 * There are library functions to change and to measure the size of a window.
 */

/**
 * SECTION:glk-window-types
 * @short_description: Blank, pair, text grid, text buffer, and graphics windows
 * @include: glk.h
 * 
 * A technical description of all the window types, and exactly how they behave.
 */

/**
 * SECTION:glk-echo-streams
 * @short_description: Creating a copy of a window's output
 * @include: glk.h
 *
 * Every window has an associated window stream; you print to the window by
 * printing to this stream. However, it is possible to attach a second stream to
 * a window. Any text printed to the window is also echoed to this second
 * stream, which is called the window's <quote>echo stream.</quote>
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
 *   stream is <quote>owned</quote> by the window, and dies with it. The echo
 *   stream is merely temporarily associated with the window.
 * </para></note>
 * 
 * If a stream is closed, and it is the echo stream of one or more windows,
 * those windows are reset to not echo anymore. (So then calling
 * glk_window_get_echo_stream() on them will return %NULL.) 
 */

/**
 * SECTION:glk-window-other
 * @short_description: Miscellaneous functions for windows
 * @include: glk.h
 *
 * This section contains functions for windows that don't fit anywhere else.
 */

/**
 * SECTION:glk-events
 * @short_description: Waiting for events
 * @include: glk.h
 *
 * As described in <link linkend="chimara-Your-Programs-Main-Function">Your
 * Program's Main Function</link>, all player input is handed to your program by
 * the glk_select() call, in the form of events. You should write at least one
 * event loop to retrieve these events.
 */

/**
 * SECTION:glk-character-input-events
 * @short_description: Events representing a single keystroke
 * @include: glk.h
 *
 * You can request character input from text buffer and text grid windows. See 
 * #evtype_CharInput. There are separate functions for requesting Latin-1 input
 * and Unicode input; see #gestalt_Unicode.
 */

/**
 * SECTION:glk-line-input-events
 * @short_description: Events representing a line of user input
 * @include: glk.h
 *
 * You can request line input from text buffer and text grid windows. See
 * #evtype_LineInput. There are separate functions for requesting Latin-1 input
 * and Unicode input; see #gestalt_Unicode.
 */

/**
 * SECTION:glk-streams
 * @short_description: Input and output abstractions
 * @include: glk.h
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
 * be, see <link linkend="chimara-Startup-Options">Startup Options</link>.
 * 
 * A stream is opened with a particular file mode, see the 
 * <code>filemode_</code> constants below.
 *
 * For information on opening streams, see the discussion of each specific type
 * of stream in <link linkend="chimara-The-Types-of-Streams">The Types of
 * Streams</link>. Remember that it is always possible that opening a stream
 * will fail, in which case the creation function will return %NULL.
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
 * Glk has a notion of the <quote>current (output) stream</quote>. If you print
 * text without specifying a stream, it goes to the current output stream. The
 * current output stream may be %NULL, meaning that there isn't one. It is
 * illegal to print text to stream %NULL, or to print to the current stream when
 * there isn't one.
 *
 * If the stream which is the current stream is closed, the current stream
 * becomes %NULL. 
 */
 
/**
 * SECTION:glk-print
 * @short_description: Printing to streams
 * @include: glk.h
 *
 * You can print Latin-1 and Unicode characters, null-terminated strings, or
 * buffers to any stream. The characters will be converted into the appropriate
 * format for that stream.
 */
 
/**
 * SECTION:glk-read
 * @short_description: Reading from streams
 * @include: glk.h
 *
 * You can read Latin-1 or Unicode characters, buffers, or whole lines from any
 * stream. The characters will be converted into the form in which you request
 * them.
 */
 
/**
 * SECTION:glk-closing-streams
 * @short_description: Closing streams and retrieving their character counts
 * @include: glk.h
 *
 * When you close a Glk stream, you have the opportunity to examine the
 * character counts &mdash; the number of characters written to or read from the
 * stream.
 */

/**
 * SECTION:glk-stream-positions
 * @short_description: Moving the read/write mark
 * @include: glk.h
 *
 * You can set the position of the read/write mark in a stream.
 *
 * <note><para>
 *   Which makes one wonder why they're called <quote>streams</quote> in the
 *   first place. Oh well.
 * </para></note>
 */

/**
 * SECTION:glk-stream-types
 * @short_description: Window, memory, and file streams
 * @include: glk.h
 *
 * <refsect2 id="chimara-Window-Streams"><title>Window Streams</title>
 * <para>
 * Every window has an output stream associated with it. This is created
 * automatically, with #filemode_Write, when you open the window. You get it
 * with glk_window_get_stream().
 * 
 * A window stream cannot be closed with glk_stream_close(). It is closed
 * automatically when you close its window with glk_window_close().
 * 
 * Only printable characters (including newline) may be printed to a window
 * stream. See <link linkend="chimara-Character-Encoding">Character 
 * Encoding</link>.
 * </para>
 * </refsect2>
 * <refsect2 id="chimara-Memory-Streams"><title>Memory Streams</title>
 * <para>
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
 * </para>
 * </refsect2>
 * <refsect2 id="chimara-File-Streams"><title>File Streams</title>
 * <para>
 * You can open a stream which reads from or writes to a disk file. See 
 * glk_stream_open_file() and glk_stream_open_file_uni().
 *
 * The file may be written in text or binary mode; this is determined by the
 * file reference you open the stream with. Similarly, platform-dependent
 * attributes such as file type are determined by the file reference. See <link
 * linkend="chimara-File-References">File References</link>.
 * </para>
 * </refsect2>
 */
 
/**
 * SECTION:glk-stream-other
 * @short_description: Miscellaneous functions for streams
 * @include: glk.h
 *
 * This section includes functions for streams that don't fit anywhere else.
 */

/**
 * SECTION:glk-fileref
 * @short_description: A platform-independent way to refer to disk files
 * @include: glk.h
 *
 * You deal with disk files using file references. Each fileref is an opaque C
 * structure pointer; see <link linkend="chimara-Opaque-Objects">Opaque 
 * Objects</link>.
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
 * You always provide a usage argument when you create a fileref. The usage is a
 * mask of constants (see below) to indicate the file type and the mode (text or
 * binary.) These values are used when you create a new file, and also to filter
 * file lists when the player is selecting a file to load. 
 * 
 * In general, you should use text mode if the player expects to read the file
 * with a platform-native text editor; you should use binary mode if the file is
 * to be read back by your program, or if the data must be stored exactly. Text
 * mode is appropriate for #fileusage_Transcript; binary mode is appropriate for
 * #fileusage_SavedGame and probably for #fileusage_InputRecord. #fileusage_Data
 * files may be text or binary, depending on what you use them for. 
 */
 
/**
 * SECTION:glk-fileref-types
 * @short_description: Four different ways to create a file reference
 * @include: glk.h
 *
 * There are four different functions for creating a fileref, depending on how
 * you wish to specify it. Remember that it is always possible that a fileref
 * creation will fail and return %NULL.
 */
 
/**
 * SECTION:glk-fileref-other
 * @short_description: Miscellaneous functions for file references
 * @include: glk.h
 *
 * This section includes functions for file references that don't fit anywhere
 * else.
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
 * If this preprocessor symbol is defined, so are all the Unicode functions and
 * constants (see #gestalt_Unicode). If not, not.
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
 * gestalt_Version:
 *
 * For an example of the gestalt mechanism, consider the selector
 * #gestalt_Version. If you do
 * <informalexample><programlisting>
 * #glui32 res;
 * res = #glk_gestalt(#gestalt_Version, 0);
 * </programlisting></informalexample>
 * <code>res</code> will be set to a 32-bit number which encodes the version of
 * the Glk spec which the library implements. The upper 16 bits stores the major
 * version number; the next 8 bits stores the minor version number; the low 8 
 * bits stores an even more minor version number, if any.
 *
 * <note><para>
 *   So the version number 78.2.11 would be encoded as 0x004E020B.
 * </para></note>
 *
 * The current Glk specification version is 0.7.0, so this selector will return
 * 0x00000700.
 *
 * <informalexample><programlisting>
 * #glui32 res;
 * res = #glk_gestalt_ext(#gestalt_Version, 0, NULL, 0);
 * </programlisting></informalexample>
 * does exactly the same thing. Note that, in either case, the second argument 
 * is not used; so you should always pass 0 to avoid future surprises.
 */

/**
 * gestalt_CharInput:
 *
 * If you set <code>ch</code> to a character code, or a special code (from
 * 0xFFFFFFFF down), and call
 * <informalexample><programlisting>
 * #glui32 res;
 * res = #glk_gestalt(#gestalt_CharInput, ch);
 * </programlisting></informalexample>
 * then <code>res</code> will be %TRUE (1) if that character can be typed by
 * the player in character input, and %FALSE (0) if not. See <link
 * linkend="chimara-Character-Input">Character Input</link>.
 */

/**
 * gestalt_LineInput:
 *
 * If you set <code>ch</code> to a character code, and call
 * <informalexample><programlisting>
 * #glui32 res;
 * res = #glk_gestalt(#gestalt_LineInput, ch);
 * </programlisting></informalexample>
 * then <code>res</code> will be %TRUE (1) if that character can be typed by the
 * player in line input, and %FALSE (0) if not. Note that if <code>ch</code> is 
 * a nonprintable Latin-1 character (0 to 31, 127 to 159), then this is 
 * guaranteed to return %FALSE. See <link linkend="chimara-Line-Input">Line
 * Input</link>.
 */

/**
 * gestalt_CharOutput:
 *
 * If you set <code>ch</code> to a character code (Latin-1 or higher), and call
 * <informalexample><programlisting>
 * #glui32 res, len;
 * res = #glk_gestalt_ext(#gestalt_CharOutput, ch, &amp;len, 1);
 * </programlisting></informalexample>
 * then <code>res</code> will be one of #gestalt_CharOutput_CannotPrint,
 * #gestalt_CharOutput_ExactPrint, or #gestalt_CharOutput_ApproxPrint (see 
 * below.)
 * 
 * In all cases, <code>len</code> (the #glui32 value pointed at by the third
 * argument) will be the number of actual glyphs which will be used to represent
 * the character. In the case of #gestalt_CharOutput_ExactPrint, this will 
 * always be 1; for #gestalt_CharOutput_CannotPrint, it may be 0 (nothing 
 * printed) or higher; for #gestalt_CharOutput_ApproxPrint, it may be 1 or 
 * higher. This information may be useful when printing text in a fixed-width 
 * font.
 *
 * <note><para>
 *   As described in <link linkend="chimara-Other-API-Conventions">Other API
 *   Conventions</link>, you may skip this information by passing %NULL as the
 *   third argument in glk_gestalt_ext(), or by calling glk_gestalt() instead.
 * </para></note>
 *
 * This selector will always return #gestalt_CharOutput_CannotPrint if 
 * <code>ch</code> is an unprintable eight-bit character (0 to 9, 11 to 31, 127 
 * to 159.)
 *
 * <note><para>
 *   Make sure you do not get confused by signed byte values. If you set a
 *   <quote><type>char</type></quote> variable <code>ch</code> to 0xFE, the 
 *   small-thorn character (&thorn;), and then call
 *   <informalexample><programlisting>
 *   res = #glk_gestalt(#gestalt_CharOutput, ch);
 *   </programlisting></informalexample>
 *   then (by the definition of C/C++) <code>ch</code> will be sign-extended to
 *   0xFFFFFFFE, which is not a legitimate character, even in Unicode. You 
 *   should write
 *   <informalexample><programlisting>
 *   res = #glk_gestalt(#gestalt_CharOutput, (unsigned char)ch);
 *   </programlisting></informalexample>
 *   instead.
 * </para></note>
 * <note><para>
 *   Unicode includes the concept of non-spacing or combining characters, which 
 *   do not represent glyphs; and double-width characters, whose glyphs take up
 *   two spaces in a fixed-width font. Future versions of this spec may 
 *   recognize these concepts by returning a <code>len</code> of 0 or 2 when
 *   #gestalt_CharOutput_ExactPrint is used. For the moment, we are adhering to 
 *   a policy of <quote>simple stuff first</quote>.
 * </para></note>
 */
 
/**
 * gestalt_CharOutput_CannotPrint:
 *
 * When the #gestalt_CharOutput selector returns this for a character, the
 * character cannot be meaningfully printed. If you try, the player may see
 * nothing, or may see a placeholder.
 */

/**
 * gestalt_CharOutput_ApproxPrint:
 *
 * When the #gestalt_CharOutput selector returns this for a character, the 
 * library will print some approximation of the character. It will be more or 
 * less right, but it may not be precise, and it may not be distinguishable from
 * other, similar characters. (Examples: 
 * <quote><computeroutput>ae</computeroutput></quote> for the one-character
 * <quote>&aelig;</quote> ligature, 
 * <quote><computeroutput>e</computeroutput></quote> for 
 * <quote>&egrave;</quote>, <quote><computeroutput>|</computeroutput></quote> 
 * for a broken vertical bar (&brvbar;).)
 */
 
/**
 * gestalt_CharOutput_ExactPrint:
 *
 * When the #gestalt_CharOutput selector returns this for a character, the
 * character will be printed exactly as defined.
 */

/**
 * gestalt_Unicode:
 *
 * The basic text functions will be available in every Glk library. The Unicode
 * functions may or may not be available. Before calling them, you should use
 * the following gestalt selector:
 * <informalexample><programlisting>
 * glui32 res;
 * res = #glk_gestalt(#gestalt_Unicode, 0);
 * </programlisting></informalexample>
 * 
 * This returns 1 if the Unicode functions are available. If it returns 0, you
 * should not try to call them. They may print nothing, print gibberish, or
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
 * you may get link-time errors. If the 
 * <filename class="headerfile">glk.h</filename> file is so old that it does not
 * declare the Unicode functions and constants, you may even get compile-time
 * errors.
 * 
 * To avoid this, you can perform a preprocessor test for the existence of
 * #GLK_MODULE_UNICODE. 
 */

/**
 * evtype_None:
 *
 * No event. This is a placeholder, and glk_select() never returns it.
 */

/**
 * evtype_Timer:
 *
 * An event that repeats at fixed intervals. See <link 
 * linkend="chimara-Timer-Events">Timer Events</link>.
 */
 
/**
 * evtype_CharInput:
 *
 * A keystroke event in a window. See <link 
 * linkend="chimara-Character-Input-Events">Character Input Events</link>.
 *
 * If a window has a pending request for character input, and the player hits a
 * key in that window, glk_select() will return an event whose type is
 * #evtype_CharInput. Once this happens, the request is complete; it is no 
 * longer pending. You must call glk_request_char_event() or
 * glk_request_char_event_uni() if you want another character from that window.
 * 
 * In the event structure, @win tells what window the event came from. @val1 
 * tells what character was entered; this will be a character code, or a special
 * keycode. (See <link linkend="chimara-Character-Input">Character 
 * Input</link>.) If you called glk_request_char_event(), @val1 will be in 
 * 0..255, or else a special keycode. In any case, @val2 will be 0.
 */

/**
 * evtype_LineInput:
 *
 * A full line of input completed in a window. See <link 
 * linkend="chimara-Line-Input-Events">Line Input Events</link>.
 *
 * If a window has a pending request for line input, and the player hits
 * <keycap>enter</keycap> in that window (or whatever action is appropriate to
 * enter his input), glk_select() will return an event whose type is
 * #evtype_LineInput. Once this happens, the request is complete; it is no 
 * longer pending. You must call glk_request_line_event() if you want another 
 * line of text from that window.
 * 
 * In the event structure, @win tells what window the event came from. @val1 
 * tells how many characters were entered. @val2 will be 0. The characters
 * themselves are stored in the buffer specified in the original
 * glk_request_line_event() or glk_request_line_event_uni() call. 
 *
 * <note><para>There is no null terminator stored in the buffer.</para></note>
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
 * A mouse click in a window. See <link 
 * linkend="chimara-Mouse-Input-Events">Mouse Input Events</link>.
 */
 
/**
 * evtype_Arrange:
 *
 * An event signalling that the sizes of some windows have changed. 
 * 
 * Some platforms allow the player to resize the Glk window during play. This 
 * will naturally change the sizes of your windows. If this occurs, then
 * immediately after all the rearrangement, glk_select() will return an event
 * whose type is #evtype_Arrange. You can use this notification to redisplay the
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
 *   different <quote>sizes</quote> in the metric of rows and columns, which is
 *   the important metric and the only one you have access to.
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
 * occurs, then glk_select() will return an event whose type is #evtype_Redraw.
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
 * events, this is platform-dependent. See #evtype_Arrange.
 *
 * For more about redraw events and how they affect graphics windows, see <link
 * linkend="chimara-Graphics-Windows">Graphics Windows</link>.
 */

/**
 * evtype_SoundNotify:
 *
 * On platforms that support sound, you can request to receive an 
 * #evtype_SoundNotify event when a sound finishes playing. See <link
 * linkend="chimara-Playing-Sounds">Playing Sounds</link>.
 */
 
/**
 * evtype_Hyperlink:
 * 
 * On platforms that support hyperlinks, you can request to receive an
 * #evtype_Hyperlink event when the player selects a link. See <link
 * linkend="chimara-Accepting-Hyperlink-Events">Accepting Hyperlink 
 * Events</link>.
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
 * The event types are described below. Note that #evtype_None is zero, and the
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
 * keycode_MAXVAL:
 *
 * This value is equal to the number of special keycodes. The last keycode is
 * The last keycode is always 
 * <informalequation>
 *   <alt>(0x100000000 - <keysym>keycode_MAXVAL</keysym>)</alt>
 *   <mathphrase>(0x100000000 - <keysym>keycode_MAXVAL</keysym>)</mathphrase>
 * </informalequation>
 * .
 */

/**
 * stream_result_t:
 * @readcount: Number of characters read from the stream.
 * @writecount: Number of characters printed to the stream, including ones that
 * were thrown away.
 *
 * If you are interested in the character counts of a stream (see <link
 * linkend="chimara-Streams">Streams</link>), then you can pass a pointer to
 * #stream_result_t as an argument of glk_stream_close() or glk_window_close().
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
 * <quote>0</quote> (zero) characters in the <quote>normal</quote> font.
 * However, the window may use a non-fixed-width font, so that number of
 * characters in a line could vary. The window might even support 
 * variable-height text (say, if the player is using large text for emphasis);
 * that would make the number of lines in the window vary as well.
 * 
 * Similarly, when you set a fixed-size split in the measurement system of a
 * text buffer, you are setting a window which can handle a fixed number of rows
 * (or columns) of <quote>0</quote> characters. The number of rows (or
 * characters) that will actually be displayed depends on font variances.
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
 * When the player finishes his line of input, the library will display the
 * input text at the end of the buffer text (if it wasn't there already.) It
 * will be followed by a newline, so that the next text you print will start a
 * new line (paragraph) after the input.
 * 
 * If you call glk_cancel_line_event(), the same thing happens; whatever text
 * the user was composing is visible at the end of the buffer text, followed by
 * a newline.
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
 * line, it goes to the beginning of the next line. The library makes no attempt
 * to wrap lines at word breaks.
 * 
 * <note><para>
 *   Note that printing fancy characters may cause the cursor to advance more
 *   than one position per character. (For example, the <quote>&aelig;</quote>
 *   ligature may print as two characters.) See <link 
 *   linkend="chimara-Output">Output</link>, for how to test this situation.
 * </para></note>
 * 
 * You can set the cursor position with glk_window_move_cursor().
 * 
 * When a text grid window is resized smaller, the bottom or right area is
 * thrown away, but the remaining area stays unchanged. When it is resized
 * larger, the new bottom or right area is filled with blanks.
 * 
 * <note><para>
 *   You may wish to watch for #evtype_Arrange events, and clear-and-redraw your
 *   text grid windows when you see them change size.
 * </para></note>
 * 
 * Text grid window support character and line input, as well as mouse input (if
 * a mouse is available.)
 * 
 * Mouse input returns the position of the character that was touched, from
 * (0,0) to 
 * <inlineequation>
 *   <alt>(width-1,height-1)</alt>
 *   <mathphrase>(width - 1, height - 1)</mathphrase>
 * </inlineequation>
 * .
 * 
 * Character input is as described in the previous section.
 * 
 * Line input is slightly different; it is guaranteed to take place in the
 * window, at the output cursor position. The player can compose input only to
 * the right edge of the window; therefore, the maximum input length is
 * <inlineequation>
 *   <alt>(windowwidth - 1 - cursorposition)</alt>
 *   <mathphrase>(windowwidth - 1 - cursorposition)</mathphrase>
 * </inlineequation>
 * . If the maxlen argument of glk_request_line_event() is smaller than this,
 * the library will not allow the input cursor to go more than maxlen characters
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
 * same thing happens.
 */
 
/**
 * wintype_Graphics: 
 * 
 * A graphics window contains a rectangular array of pixels. Its size is the
 * number of columns and rows of the array.
 * 
 * Each graphics window has a background color, which is initially white. You
 * can change this; see <link 
 * linkend="chimara-Graphics-in-Graphics-Windows">Graphics in Graphics 
 * Windows</link>.
 * 
 * When a text grid window is resized smaller, the bottom or right area is
 * thrown away, but the remaining area stays unchanged. When it is resized
 * larger, the new bottom or right area is filled with the background color.
 * 
 * <note><para>
 *   You may wish to watch for #evtype_Arrange events, and clear-and-redraw your
 *   graphics windows when you see them change size.
 * </para></note>
 * 
 * In some libraries, you can receive a graphics-redraw event (#evtype_Redraw)
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
 * see <link linkend="chimara-Graphics-in-Graphics-Windows">Graphics in Graphics
 * Windows</link>.
 * 
 * Graphics windows support no text input or output.
 * 
 * Not all libraries support graphics windows. You can test whether Glk graphics
 * are available using the gestalt system. In a C program, you can also test
 * whether the graphics functions are defined at compile-time. See <link 
 * linkend="chimara-Testing-for-Graphics-Capabilities">Testing for Graphics
 * Capabilities</link>. 
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
 * winmethod_DirMask:
 *
 * Bitwise AND this value with a window splitting method argument to find
 * whether the split is #winmethod_Left, #winmethod_Right, #winmethod_Above, or
 * #winmethod_Below.
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
 * winmethod_DivisionMask:
 *
 * Bitwise AND this value with a window splitting method argument to find
 * whether the new window has #winmethod_Fixed or #winmethod_Proportional.
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
 * character codes may be converted to Latin-1. 
 *
 * <note><para>
 *   Line breaks will always be converted; other conversions are more
 *   questionable. If you write out a file in text mode, and then read it back
 *   in text mode, high-bit characters (128 to 255) may be transformed or lost.
 * </para></note>
 * <note><title>Chimara</title>
 * <para>
 * Text mode files in Chimara are in UTF-8, which is GTK+'s native file
 * encoding.
 * </para></note>
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
 * type is #fileusage_SavedGame, #fileusage_Transcript, #fileusage_InputRecord,
 * or #fileusage_Data.
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
 *   Corresponds to mode <code>"a"</code> in the stdio library, using fopen().
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

