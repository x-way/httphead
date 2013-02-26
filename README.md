<h1>httphead</h1>

<h2>Synopsis</h2>

<p>httphead [<var>options</var>] <var>url</var></p>

<h2>Description</h2>

<p>Send a customized HTTP-Request to <var>url</var> and display the HTTP-Response headers.</p>

<h2>Options</h2>

<dl>
<dt>-r</dt>
<dd>display also the HTTP-Request</dd>
<dt>-q</dt>
<dd>display only the recieved HTTP status code</dd>
<dt>-n</dt>
<dd>do not send a User-Agent header with the HTTP-Request</dd>
<dt>-u <var>agent-string</var></dt>
<dd>use <var>agent-string</var> as value for the User-Agent header</dd>
<dt>-a <var>accept-string</var></dt>
<dd>send an Accept header with the value <var>accept-string</var></dd>
<dt>-e <var>encoding-string</var></dt>
<dd>send an Accept-Encoding header with the value <var>encoding-string</var></dd>
<dt>-c <var>charset-string</var></dt>
<dd>send an Accept-Charset header with the value <var>charset-string</var></dd>
<dt>-l <var>language-string</var></dt>
<dd>send an Accept-Language header with the value <var>language-string</var></dd>
</dl>

<dl>
<dt>-v</dt>
<dd>display the version of httphead</dd>
<dt>-h</dt>
<dd>display a help message</dd>
<dt>-b</dt>
<dd>display the BSD license</dd>
</dl>

<h2>Requirements</h2>

<p>Unix-like platform and C compiler.</p>

<h2>Download</h2>

<p><a href="httphead.c">httphead.c</a></p>


<h2>Bugs</h2>

<p>None known yet. If you find one please report it in the issue tracker.</p>

<h2>Future directions</h2>

<p>Support for Digest HTTP-Auth and HTTPS.</p>

<h2>See also</h2>

<ul>
<li><a href="http://www.gnu.org/software/wget/">wget</a> if you want to download more than the headers.</li>
</ul>

<h2>License</h2>

<p>httphead is released under the BSD license.</p>

<h2>Copyright</h2>

<p>httphead is &copy; 2006 <a href="http://x-way.waterwave.ch/">Andreas Jaggi</a></p>
