In order to add webui files to te ressource catalog file (webui.qrc)
either use the "Add existing files..." in qtcreator or generate appropriate lines
using find and bash:

```
$ for file in `find webui -type f` ; do echo "<file>$file</file>" ; done
```

Place the output of this between the `<qresource prefix="/">` and its
closing tag.
The default prefix of `/` plus the `webui` folder rpefix make the files
available under the `:/webui` ressource path where the static file server
is set to look for them. The index file of any folder must end with `.html`
at this point as the server will not try other names.
