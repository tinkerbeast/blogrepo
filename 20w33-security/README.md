Security
=========

Common attack patterns
----------------------

### Community driven atack pattern list and classifications

Note: These are not specific exploits like metasploit, but general attacks

https://capec.mitre.org/
https://cwe.mitre.org/
https://owasp.org/

Note: CAPEC nicely classifies common threats


### Most common attacks - XSS


### Most common attacks - Protocol vulnerabilites


### Most common attacks - Buffer overflow


### Most common attacks - DoS


### Most common attacks - Social engineering


### Hardware security


### Tools

* Nessus
* metasploit
* IBM appscan
* Checkmarx
* codenomicon
* hping
* nmap
* isic.sourcefourge.net


### CVSS

CVSS assigns a single score to a vulnerability.

https://en.wikipedia.org/wiki/Common_Vulnerability_Scoring_System
https://www.first.org/cvss/specification-document
https://www.first.org/cvss/calculator/3.1


### Security terms

CIA - confidentiality-integrity-availability
Non-repuditation - Actions on information can be attributed to a person/entity
Authentication - 
Authorisation - 
Attack vector
Attack surface


Insecure coding in C
--------------------

### General principles

* Be aware of illegal, undefined and loosely defined behaviour
	* Sizeof primitives
	* Commit points and side effect operators
	* Portable variables types
	* Portable alignment
	* Compiler optimisation and order of statements (For hardware side effects)
* Stack buffer overflow
	* Writing exploits (eg. just piping in input to a application doing gets)
	* Arc injection (eg. return to lib-c)
	* Code injection (eg. system() call and shell code)
	* PCB/TCB modification
	* Return oriented programme	(https://resources.infosecinstitute.com/return-oriented-programming-rop-attacks/#gref)
* Hardcoded values
	* Can always be disassembled and read	
* Securing your programme
	* Writ and Execute mutually exclusive (No execute bit)
	* Page boundaries
	* ASLR
	* Stack protect (stack canaries)

Book: Secure coding in C and C++

### Safe C

strcpy -> strncpy -> strlcpy/strcpy_s
strcat -> strncat -> strlcat/strcat_s -strtok
sprintf -> snprintf
vsprintf -> vsnprintf
gets -> fgets/gets_s
makepath -> _makepath_s (MSDN)
_splitpath -> _splitpath_s (MSDN)
scanf/sscanf -> sscanf_s (MSDN)
snscanf -> _snscanf_s (MSDN)
strlen -> strnlen_s (MSDN)


OS hardening
------------

### Linux kernel hardening

* Strict kernel memory permission
	* stack protector
	* heap protector
	* counter integrity
	* CONFIG_CC_STACKPROTECTOR, CONFIG_CC_STACKPROTECTOR_STRONG
	* CONFIG_HARDENED_USERCOPY, CONFIG_HARDENED_USERCOPY_FALLBACK=false
* Reduced access to syscalls
* Restricy access to kernel modules
	* CONFIG_STRICT_KERNEL_RWX, CONFIG_STRICT_MODULE_RWX
	* CONFIG_SECCOMP, CONFIG_SECCOMP_FILTER
	* CONFIG_MODULE_SIG, CONFIG_MODULE_SIG_FORCE
	* CONFIG_DEVKMEM=false
* Kernel Addess space randomisation
	* CONFIG_RANDOMIZE_BASE
	* CONFIG_RANDOMIZE_MEMORY
* dmesg limits
	* CONFIG_SECURITY_DMESG_RESTRICT

* Secure boot
* Signed kernel module
* ASLR
* XPSPACE
* OSC

https://kernsec.org/wiki/index.php/Kernel_Self_Protection_Project/Recommended_Settings

### Kernel hardening tools

* Corona
* Ktest
* dASLR
* Xcheck

### SE Linux

TODO

### OS - Secure hardware

Hardware RNG https://lwn.net/Articles/648550/

Application level hardening
---------------------------

### Services check 

* https://www.cisecurity.org/cybersecurity-tools/cis-cat-pro/cis-cat-faq/
* https://www.qualys.com/cloud-platform/

### Cloud Security areas

* APIs
	* Appropriate autharisation (OAuth, SSO, etc) + authentication (role, acl, etc)
	* Secure password handling (hashing, etc)
	* Secure session management
	* How to secure third party APIs? Treat them similar to libraries		
* Data handling
	* See data taxnomy
	* Credential store (eg. HashiCorp Vault)
	* Keystore (AWS KMS)
	* DB - encrypted connection, secure authetication+authrisation
* Logging
	* No PII logged
	* Log retention policies well defined
	* Security event monitoring
* Vulnerability management
	* OWASP top 10

### App monitoring

* Infra / platform level
	* Audit monitor (auth related monitoring)
	* Usage monitor
	* Network forensics
	* Traffic monitor
* App level monitoring
	* ...
* App logs
	* TODO log necessary for security	


Secure development
------------------

### Dev and testing methodology

Threat landscape = threats * devices * attackers

* Threat model during design
* Static codde analysis during dev
* Fuzz testing using http://www.codenomicon.com/index.html

### Secure development in linux

* Updates
	* Update to patch security vulnerabilites
	* Updating components may lead to new vulnerabilites
* Eliminate unecessary components - gcc, gdb, etc
* Basics in coding
	* Input validation
	* Safe C libraries
	* gcc -fPIE -pie : For ASLR code
	* gcc -D_FORTIFY_SOURCE=2 -O2 : For fortify source (OSC) https://access.redhat.com/blogs/766093/posts/1976213
	* ASIG x7F
	* Do not launch a shell program or shell command using system(), execve() etc
	* Every time you use hashing, is the code you're writing secure?
	* Log safely
* Do static analysis
* 3rd party libs
	* Remove unnecessary 3rd party libs
	* Check for hardening in 3rd party libs
* Network input
	* Minmise listeners
	* Only SSL/TLS listenrs
	* Listeners without root privilege
	* Firewall as necessary
* Logging in to system
	* Make sure PAM has right cryptographic settings
	* Make sure PAM users have righ login shell
* Privileges as necessary	
	* Do no run as processes as root / privileged user
	* setuid, setgid, setsid is dangerous
	* Do not sudo
	* setcap, ulimit command for right privileges
* Namespace/SELinux for setting boundaries (even chroot does it)

### In case you have to write your own protocol

* Always use standards - Never design your own
* Focus on secure interoperatibilty, not just interoperatibility
* Validate peer identity, (authenticated identity - certificates, root ca etc)
* Use a strong RNG, preferably hardware (See pitfall article https://arstechnica.com/information-technology/2012/02/crypto-shocker-four-of-every-1000-public-keys-provide-no-security/)
* Proper key store
* Test your protocols to break them (Standard tools, Fuzz testing, Flood, error injection, etc)


### Key mangement

SWIMS: https://en.wikipedia.org/wiki/System_Wide_Information_Management

https://www.cisco.com/security/pki/
https://www.cisco.com/c/m/en_us/products/cloud-systems-management/dna-center/use-case-software-image-management-provisioning.html


Data taxonomy
-------------

### Data lifecycle

* collection/creation : categorize / classify, apply controls, consent etc
* usage : data can only be used for the original purpose it was collected
* sharing: permissions and controls
* retention: 
* destruction

Note: Data processing is entire lifecycle

### Legal terms

data subject: the person to which personally-identifiable-information can be applied
data controller: the entity who takes all the decisions about the data, responsible for violations
data processor: the enitity which actually processes the data under control from controller, not responsible as long as following controller

data residency: the physical location of data
data localisation: process user data in a datacenter that is physically situated in same country as origin of data
data sovereignty: concept that information stored in digital form is subjected to laws of country where it physically resides
data portability: of things concerning the right to receive/transfer personal data

### Data roles

* data owner: similar to controller, derermins classificaion, protection requirements
* data trustee: similar to controller, ensures protection and proper access of the data
* data custodian: similar to processor, mantains administrative controller
* data user: uses data

### Data categories

administrative, telemetry, support, ops, strategic, hr, etc

### Data classification

* public (no misuse potential)
* confidential (misuse impact to self only, not that significant)
* highly-confidential (misuse impacts self, partners, customers, not that significant)
* restricted (misuse leads to significant impact to self, partners, customers)
