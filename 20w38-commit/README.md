Git commit messages
-------------------

https://chris.beams.io/posts/git-commit/

### Separating commits

If you are commiting more than one idea (each idea can be capture by the subject line), then separate out the commits.

### Baisc format

Subject line // 50 chars or less, avoid markdown, don't end with full-stop, must start with capital letter
// empty line
More detailed explanatory text, if necessary. Wrap it to about 72 // body lines, 72 chars or less
characters or so. In some contexts, the first line is treated as the // can have markdown syntax
subject of the commit and the rest of the text as the body. The
blank line separating the summary from the body is critical (unless
you omit the body entirely); various tools like `log


Resolves: #123         // issue tracker references at bottom
See also: #456, #789


### Subject line - imperative mood usage - Why????????????

Use: Clean the Http module
Instead-of: Cleaned the Http mode

Essentially try to re-fit the subject line as - 

If applied, this commit will <subject-line>


### Subject line - format

<Action> <Moduels affected> <optional reason>

Action: feature, fix, style, refactor, test, document, baseline,                                       

Eg.  If applied, this commit will || Feature  | the badunifex and prototype module | for supporting then syntax
Eg.  If applied, this commit will || Fix  | the badunifex module | for bugid XXX
Eg.  If applied, this commit will || Style  | the concepts.h file in badunifex module | as per STYLGUIDE.md
Eg.  If applied, this commit will || Refactor  | the prototype/prototyp2.h file | to properly use badunifex
