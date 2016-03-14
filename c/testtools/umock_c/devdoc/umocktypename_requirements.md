#umocktypename requirements
 
#Overview

umocktypename is a module that provides a way to bring C type names to a normalized form in order to ensure that type name comparisons can be made easily.
For example "const                   char \*" would be brought to "const char\*".

#Exposed API

```c
extern char* umocktypename_normalize(const char* typename);
```

##umocktypename_normalize

```c
extern char* umocktypename_normalize(const char* typename);
```

**SRS_UMOCKTYPENAME_01_001: [** umocktypename_normalize shall return a char\* with a newly allocated string that contains the normalized typename. **]**
**SRS_UMOCKTYPENAME_01_002: [** umocktypename_normalize shall remove all spaces at the beginning of the typename. **]**
**SRS_UMOCKTYPENAME_01_003: [** umocktypename_normalize shall remove all spaces at the end of the typename. **]**
**SRS_UMOCKTYPENAME_01_004: [** umocktypename_normalize shall remove all extra spaces (more than 1 space) between elements that are part of the typename. **]**
For example "const  char" would be normalized to "const char". Also "char \*" would be normalized to "char\*".
