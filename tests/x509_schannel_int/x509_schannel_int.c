// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Integration tests for x509_schannel, in particular that x509_verify_certificate_in_chain()
// correctly determines whether certificates chain up to trusted certs or not.  There are
// no networking calls or changes to local machine's certificate store, so it runs 
// without need for a server on other side of test or for special privileges on host machine.

#ifndef WIN32
#error x509_schannel_unittests can only be compiled and used in Windows
#else

#include "windows.h"

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif
#include "testrunnerswitcher.h"

#include <wincrypt.h>
#include <ncrypt.h>
#include <bcrypt.h>
#include <wchar.h>

#include "azure_c_shared_utility/x509_schannel.h"


#define TEST_MAX_SERVER_CERTIFICATE_ENCODE_SIZE 4096

// NOTE: This certificate is for test purposes only and must not be used in any production environment.
// This certificate is a root cert that in this test our simulated client certificates chain up to.
#define X509_TEST_CERTIFICATE_CHAIN1 \
"-----BEGIN CERTIFICATE-----""\n" \
"MIIDojCCAoqgAwIBAgIQZ4iO7pTqsrtEub7360KODTANBgkqhkiG9w0BAQsFADA6""\n"  \
"MTgwNgYDVQQDDC9BenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5lbCBpbnRlZ3Jh""\n"  \
"dGlvbiB0ZXN0czAgFw0xODExMjgyMzA0MDVaGA8yMDY4MTExNTIzMTQwNVowOjE4""\n"  \
"MDYGA1UEAwwvQXp1cmUgQy1VdGlsaXR5IHg1MDkgc2NoYW5uZWwgaW50ZWdyYXRp""\n"  \
"b24gdGVzdHMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDB8Qk1R0fU""\n"  \
"l6nQLnfS9IBIvPxWKIbsVc87JCvgjW8FvX9o6+NadFSd7v4bvmIxk5Z9W9agMABT""\n"  \
"iT/KLIYzmJ/Afn+7moi9M+BF/sE7Mu+Hm4LVKZTbbaW5OQ1ySOFVpneoj4GyXXn8""\n"  \
"yYRspav0aUBaLyOGWyG4UUKXgrPa0jiEDmcagmrtfCr3TyowIqzb4Xhvoe9Lqqt4""\n"  \
"s8l2KHHAf4sGqye+pcC2EVWmcx1XsS97u8ngOOSSMe7pA4KTJ8+dbWfYa2kPqhle""\n"  \
"cRIeRBm7PrZA6nINT+uRQmilE+ZuuLgHoCRpDXqgJGv/we5Frhe4g2rPPORSsyv7""\n"  \
"lbBSCOzbbHfNAgMBAAGjgaEwgZ4wDgYDVR0PAQH/BAQDAgIEMB0GA1UdJQQWMBQG""\n"  \
"CCsGAQUFBwMCBggrBgEFBQcDATA6BgNVHREEMzAxgi9BenVyZSBDLVV0aWxpdHkg""\n"  \
"eDUwOSBzY2hhbm5lbCBpbnRlZ3JhdGlvbiB0ZXN0czASBgNVHRMBAf8ECDAGAQH/""\n"  \
"AgEMMB0GA1UdDgQWBBQLDpN3UOpuCRNiQGscPM7a2egIGjANBgkqhkiG9w0BAQsF""\n"  \
"AAOCAQEAOn9hb0+2Z7zKHBUtAnSBcTRClOhHlyk6BYqyhbn56FQuZY+8WfNTJsWp""\n"  \
"gJCiEqUz5GbBzEVrlxdUSfu7qAISQjqhAN8lndJ7Ux7V6IPFSzVVfqv+vkHRJREV""\n"  \
"L8gQJfBhcjqcKgB1fNH3dVk+rnNhqZEfdTayamlgCCqiNpl26PbtFLM0wjFGwVjH""\n"  \
"1f2SJ4EIGPgiFZj0k7IIpRb3eDBJ37bA2cfRqD4bscR2rAPn8jZDrE3UCIuQXSYS""\n"  \
"ama9/loDlT4qyA/g/YHKMKlI8iTv98/k7gG6TL7Tep6N9BDEcy/NH8cZPjSxi94q""\n"  \
"4CJkC6ZhNoDZ8WsCIGsoJqa3cGktAg==""\n"  \
"-----END CERTIFICATE-----"


// NOTE: This certificate is for test purposes only and must not be used in any production environment.
// This certificate is a legit root cert, but none of the test certificates chain up to it.
#define X509_TEST_CERTIFICATE_CHAIN2 \
"-----BEGIN CERTIFICATE-----""\n" \
"MIIDwzCCAqugAwIBAgIQGF/+e0JWT55NwcioWwYK4TANBgkqhkiG9w0BAQsFADBF""\n" \
"MUMwQQYDVQQDDDpBenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5lbCBpbnRlZ3Jh""\n" \
"dGlvbiB0ZXN0IChubyBjaGFpbjIpMCAXDTE4MTEyOTE2NDM1OVoYDzIwNjgxMTE2""\n" \
"MTY1MzU5WjBFMUMwQQYDVQQDDDpBenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5l""\n" \
"bCBpbnRlZ3JhdGlvbiB0ZXN0IChubyBjaGFpbjIpMIIBIjANBgkqhkiG9w0BAQEF""\n" \
"AAOCAQ8AMIIBCgKCAQEA31oJhWVlg2IlNv/WNHqu/pg4hqMvsWGLLE6rzrxnoCVi""\n" \
"Z0I3uAD+vHm5C3Eg5p3mq1zv1vpXxFDdV6fL83XPLXq+dgQp8xdj8MtnKE/RVSsJ""\n" \
"reH+u/xXnwd8fzu8Q7l4qvU3nTJk3EtkwkVxM5vP5Gg7MnHx2JHdxeW3y1PdFiOV""\n" \
"1uKi19iz+n5FhKBmLeuPd1G3pXA1r+MU/saVloGtLDmH8+YiMnDP4iHLLU6yWK7r""\n" \
"c4HW4sdR9209zddN4YlKT0MgtJlKkZaOWowThmyubomsuaAPTB5WCW7qTFcNS9Eu""\n" \
"N/GXYUbHdqWggeiJK5y2ELnwB0p+QOZwL/ir+SG9PQIDAQABo4GsMIGpMA4GA1Ud""\n" \
"DwEB/wQEAwICBDAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwRQYDVR0R""\n" \
"BD4wPII6QXp1cmUgQy1VdGlsaXR5IHg1MDkgc2NoYW5uZWwgaW50ZWdyYXRpb24g""\n" \
"dGVzdCAobm8gY2hhaW4yKTASBgNVHRMBAf8ECDAGAQH/AgEMMB0GA1UdDgQWBBT7""\n" \
"ekxNZopXaL2cfcEBXWiwEwHrGTANBgkqhkiG9w0BAQsFAAOCAQEAt8/jfQQ2GhPT""\n" \
"FQk+7IjYscNZLff6v67hXIUKdhk4kcEu9uErIRexMKv+XCFRXFyR/DZ620YSlyXQ""\n" \
"+99vmnHYYoNMAeD6v15p1yJ7/M6lUOXvJ4HHTpvngOFt6pNXwVzDJ94Bd/FodZQb""\n" \
"DuhYvA85B5aavUv98a+KzCIzaQ3gmiR1D+4Jj62n4VtyQC1ZamkclLk2DG/TCAKF""\n" \
"RLrpfu2PsFjnHS+RBgvTggSBksGucX5TmkgRAsYgocr6pnjqVfv7Rd1tC+TLygV8""\n" \
"MtlqCMFjiRs8LIu9DWaF+Wf6z4QGVGhIDW4W+utcE7ORc4FPuKuufxNs5quVhTR6""\n" \
"N/MOypAmlA==""\n" \
"-----END CERTIFICATE-----"

// NOTE: This certificate is for test purposes only and must not be used in any production environment.
// This certificate is a legit root cert, but none of the test certificates chain up to it.
#define X509_TEST_CERTIFICATE_CHAIN3 \
"-----BEGIN CERTIFICATE-----""\n" \
"MIIDwzCCAqugAwIBAgIQJSMG/xxtuYtKc/9ipIkzqDANBgkqhkiG9w0BAQsFADBF""\n" \
"MUMwQQYDVQQDDDpBenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5lbCBpbnRlZ3Jh""\n" \
"dGlvbiB0ZXN0IChubyBjaGFpbjMpMCAXDTE4MTEyOTE2NDUwOVoYDzIwNjgxMTE2""\n" \
"MTY1NTA5WjBFMUMwQQYDVQQDDDpBenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5l""\n" \
"bCBpbnRlZ3JhdGlvbiB0ZXN0IChubyBjaGFpbjMpMIIBIjANBgkqhkiG9w0BAQEF""\n" \
"AAOCAQ8AMIIBCgKCAQEAvwHMSrmTvqGHlkrImSW7Zla8b0aqaPjjzuBg993dzOQM""\n" \
"appCFotZBhvnapZ1gyYPRLfFmyRW2g5PPpIhvA02DT+WyrA0RNtxrn0dtDmlKk4E""\n" \
"uEU4pGXyAOw9O0TrrSuiyOw/BcRTSHgWSkeKiaDpP1oGV7fI+Gb77hHZzP9gWSUg""\n" \
"kVlLFQnz92X8UPGEqvnkZF06rRmtgWXJ8L+GvZBdw2DiKutdQdXja0eusbtCo16n""\n" \
"hn17+VjbXyJq4D0wPQd2BlsTxNarWGMOBdzigd36WLz5F6QI40k4dYzMHSV7lfNk""\n" \
"nn9jYON04N7GsVZiDEQW8/dte6dw/GoO14iBv4xInQIDAQABo4GsMIGpMA4GA1Ud""\n" \
"DwEB/wQEAwICBDAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwRQYDVR0R""\n" \
"BD4wPII6QXp1cmUgQy1VdGlsaXR5IHg1MDkgc2NoYW5uZWwgaW50ZWdyYXRpb24g""\n" \
"dGVzdCAobm8gY2hhaW4zKTASBgNVHRMBAf8ECDAGAQH/AgEMMB0GA1UdDgQWBBRO""\n" \
"+UANd6vHbVPXyMx/C3sBL7bsLjANBgkqhkiG9w0BAQsFAAOCAQEAp5UUFJB7hK7+""\n" \
"lpt4wDYJ+8ao8+a0WNtNwQf/IxLU6Mi6o39uMIZaJhXjlL8bf7cVIschOBEeWxSZ""\n" \
"4HvP8FZxvNo+WsmZtteFOhRwI4Aoa9xebmdcgU0HmF5IqsUu06xKgTeFwQYCfSjt""\n" \
"rqGldWEtfaiIX+GAp6AO5AIPi8ScGwq3WDCbLp2CSJ35i8EzFF86uF6nqt1I9cYE""\n" \
"P33C5qDlgPV9/pjxSsjeeuL1p2BHt17hFNMX40CnS2uuabOiejRKvNrORTr4FDxN""\n" \
"FdJEWbZ1DAvf/Vxx2u36LIuACGeCMhvWH0G3fdMchqcfQRhdNIDco62U42/98CRP""\n" \
"cFyISsIRMg==""\n" \
"-----END CERTIFICATE-----"

// NOTE: This certificate is for test purposes only and must not be used in any production environment.
// This certificate is a legit root cert, but none of the test certificates chain up to it.
#define X509_TEST_CERTIFICATE_CHAIN4 \
"-----BEGIN CERTIFICATE-----""\n" \
"MIIDwzCCAqugAwIBAgIQWetmG6MFzotE5ijvB0TaQzANBgkqhkiG9w0BAQsFADBF""\n" \
"MUMwQQYDVQQDDDpBenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5lbCBpbnRlZ3Jh""\n" \
"dGlvbiB0ZXN0IChubyBjaGFpbjQpMCAXDTE4MTEyOTE3Mjc0NFoYDzIwNjgxMTE2""\n" \
"MTczNzQ0WjBFMUMwQQYDVQQDDDpBenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5l""\n" \
"bCBpbnRlZ3JhdGlvbiB0ZXN0IChubyBjaGFpbjQpMIIBIjANBgkqhkiG9w0BAQEF""\n" \
"AAOCAQ8AMIIBCgKCAQEAvlVWHCBFYzk3V/mBMYi9MCaWdU+n2GgBTVh/00oFp1/o""\n" \
"R71qJsqVqfkOWMh7PwMi2IweETW/2w/b1qcUDWhLGfH2rQALizsIcIXXfTptIXHg""\n" \
"wSFhrGa842EoDHVXfejDoZToacLij1iu/YlNWqraW57iujdt9OL8o1atLR8413wM""\n" \
"wS8CC/oWI1/UfHJK8Mw8GcJJNfBC/oKWAcEcxdvX9npn/5R35/MgRqehtpged7/0""\n" \
"RMeONkQ8YWHxdjAqUIxnSzDEL+kq/oFHskN1oFpKBmIlWe/ZZAalFs7x4vC3Lonz""\n" \
"9knz6KZH5qEGWVKOr9rCwtmPvMO8t6Jh+Rr80cg44QIDAQABo4GsMIGpMA4GA1Ud""\n" \
"DwEB/wQEAwICBDAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwRQYDVR0R""\n" \
"BD4wPII6QXp1cmUgQy1VdGlsaXR5IHg1MDkgc2NoYW5uZWwgaW50ZWdyYXRpb24g""\n" \
"dGVzdCAobm8gY2hhaW40KTASBgNVHRMBAf8ECDAGAQH/AgEMMB0GA1UdDgQWBBQj""\n" \
"+JrZbDPGFbBAnJHA5SnhFljtZjANBgkqhkiG9w0BAQsFAAOCAQEAN6jtbDX1YW70""\n" \
"4drtz4xxbRCVsHrXy7RcdfEeqNipgvzX4C5x6twcCULXGlm/s5SjwaR0UfE6mZFA""\n" \
"2gb50R3q8XOdEqoAMJYgjerwuUr38SohzpSeMaWjTwWLFhVkxW5vt/pv6f2/HrTN""\n" \
"Z8qQectIljP0oeVBhouvmXGPfZpYWM7ckS4mx+FocAtX6gWGufXhiOulchsjT1HV""\n" \
"E9hYUiyCa+0u9AHBuhdipzGeZV2KnIBW9QMQR0OMIlwhuVyQz5GwXuHus2BnVsgg""\n" \
"7tO6HHkgddwJhk+3kyDy12MING6MPF45g2yvMoJYl2fviua5mlFVx6g1wrxeS0Cc""\n" \
"g+y5cZsf/Q==""\n" \
"-----END CERTIFICATE-----" \


// NOTE: This certificate is for test purposes only and must not be used in any production environment.
// Simulated server certificate that is chained up to X509_TEST_CERTIFICATE_CHAIN1
const char* x509_test_server_certificate = 
"-----BEGIN CERTIFICATE-----""\n"
"MIIDeDCCAmCgAwIBAgIQTw2XxN6ct5VDfdcMxzzcBDANBgkqhkiG9w0BAQsFADA6""\n"
"MTgwNgYDVQQDDC9BenVyZSBDLVV0aWxpdHkgeDUwOSBzY2hhbm5lbCBpbnRlZ3Jh""\n"
"dGlvbiB0ZXN0czAgFw0xODExMjgyMzA1MDNaGA8yMDY4MTExNTIzMTUwM1owFjEU""\n"
"MBIGA1UEAwwLdGVzdC14NTA5LTEwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK""\n"
"AoIBAQCuNtGq7eU1rijE7kkjpQY763K09I54XScDM7N2XHsTtDYG6L1khqbovgZ0""\n"
"mJ0uhCRjYBUi4yJYaPRdvKQx5taDRzf3GZ4rvpYwBdWRcor/o4k9iDzZGj9qnnP1""\n"
"8cVwVaLbcnOwRWJYyEPznfTcCFG2LHOIF3iFdcohKdY8iALLacwU95dYywgDWWgx""\n"
"sv2Q8C+maKjik38s3en5QQI+lHKKEXfnTYkWtPqtejXGakrOHJWcnF8tjQi6hMMe""\n"
"sZb1/PL1oeYDEpeQguNe0VznqDG0mHUeBGbyNNLBnkd12MmgxBF32cEC+INVo+Wn""\n"
"S2Gp0h2RfK4Vt6ovAKxhQzzyVxuRAgMBAAGjgZswgZgwDgYDVR0PAQH/BAQDAgWg""\n"
"MBYGA1UdEQQPMA2CC3Rlc3QteDUwOS0xMB0GA1UdJQQWMBQGCCsGAQUFBwMCBggr""\n"
"BgEFBQcDATAPBgNVHRMBAf8EBTADAgEAMB8GA1UdIwQYMBaAFAsOk3dQ6m4JE2JA""\n"
"axw8ztrZ6AgaMB0GA1UdDgQWBBS+QQFsY+Ob8IxaayQ9wl0uuJXsFDANBgkqhkiG""\n"
"9w0BAQsFAAOCAQEAgexxPxV/EXwnDaEAdnrKQfyE5/vtElitWaeCsQij0m8tbPCj""\n"
"jNHOUAM3XwrQNoffk5zW9o/njrdWcMFYcW6SyS9npBKuoCOSKDYV5p+fPluFzgie""\n"
"0BWlr8oZLpuqKNf5yQE7p6RdHWguqmSruY6HyfOjjwWtkUQSeXbA9y3qr5rypVaQ""\n"
"zKsPi9h7CkUsoTFnUFZA+UkIfoJZ+Y+7d2bfg/u6LTRsnURlr5E++Jn1zlUYUl0r""\n"
"/WwcRPj80Swd8ntifdBCx/XwJ53tWGXFx1xopmNamSPAsfwEvpFstWZFyxmCcuN5""\n"
"NqMEOiICezJDDG3q/THL/lQANjVo4QFMMdmDZA==""\n"
"-----END CERTIFICATE-----";

// Certificate/private key pairs used to check that a private key supplied in each supported
// encoding is decoded, imported and actually bound to the certificate. Generated offline for
// this test only; they are self-signed, carry no trust and must not be used anywhere else.
#define X509_TEST_RSA_CERTIFICATE \
"-----BEGIN CERTIFICATE-----""\n" \
"MIIDRTCCAi2gAwIBAgIUXzkoRpVhLGSK+eX09kUXpkwnwaowDQYJKoZIhvcNAQEL""\n" \
"BQAwMTEvMC0GA1UEAwwmYXp1cmUtYy1zaGFyZWQtdXRpbGl0eSB4NTA5IHRlc3Qg""\n" \
"KFJTQSkwIBcNMjYxMDA2MDMzNTEwWhgPMjEyNjA5MTIwMzM1MTBaMDExLzAtBgNV""\n" \
"BAMMJmF6dXJlLWMtc2hhcmVkLXV0aWxpdHkgeDUwOSB0ZXN0IChSU0EpMIIBIjAN""\n" \
"BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzTIT8j9r5vBkaSm4WfOvZp3wJvzI""\n" \
"JeOuyq809IoqAxnu9GSq1/rWSGCFBpS3iS6LsT9uQyIJHe8o2et9yGVBt4FPf4PT""\n" \
"J1wAblmIykYnrZ7W9CNO0Me4iCcXLY9KnAAAcISmcybYDkh8Nj+xNcr4D86dpRyC""\n" \
"CRU5AWSJKO1Qi2hYXTKzM+4jAHy5awJXc3pPJdDbnwjuRzsfwBTQIsK4srSro6aA""\n" \
"dG6pnJHQ5bvkJvE1DwwU64+GdsThgvHT7hTkYsXuToy7Jf19GIMAcYeT8DcoYRTr""\n" \
"qUWqect1Cz3fu+HkYcJqamqKqC+Ux2k9mi+QtDqZtPNuj1ZrE9wsMQ7RRQIDAQAB""\n" \
"o1MwUTAdBgNVHQ4EFgQU/dWLwJn08xj8z0asT2nAL6VwYjEwHwYDVR0jBBgwFoAU""\n" \
"/dWLwJn08xj8z0asT2nAL6VwYjEwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0B""\n" \
"AQsFAAOCAQEAi/0VDQyre9/3f/Ie3lnMzc8QCTC3JAw7kkcEe77K9B5wzliD/oF9""\n" \
"D0iH8TyVBdV12aZL7w9ZgGNbF+JqioLMbV909i1A5Djz71DaI6046kcHBaT2xv90""\n" \
"vbvDYB+/bW8ok0ej3CBjgtHAa19MFm9I5rrxiH0xO3NmRXW8/RvOcYD9opJ8/TSi""\n" \
"e9mPaWOQ3S5jjvF4pRoogj3IqEaifffyIxO6OuofHuqKz8j5JVepxHoOGmVCD9g7""\n" \
"aZVzGBoeNn8b1eovV3dLIhAGUKE381EIGfbciW13YBQW93+UUbPgNR9lw0AfbfFI""\n" \
"uNNH4ogozQ/OcaAMxTbiGaBNQU4MWp4+Jw==""\n" \
"-----END CERTIFICATE-----"

#define X509_TEST_RSA_PRIVATE_KEY_PKCS1 \
"-----BEGIN RSA PRIVATE KEY-----""\n" \
"MIIEogIBAAKCAQEAzTIT8j9r5vBkaSm4WfOvZp3wJvzIJeOuyq809IoqAxnu9GSq""\n" \
"1/rWSGCFBpS3iS6LsT9uQyIJHe8o2et9yGVBt4FPf4PTJ1wAblmIykYnrZ7W9CNO""\n" \
"0Me4iCcXLY9KnAAAcISmcybYDkh8Nj+xNcr4D86dpRyCCRU5AWSJKO1Qi2hYXTKz""\n" \
"M+4jAHy5awJXc3pPJdDbnwjuRzsfwBTQIsK4srSro6aAdG6pnJHQ5bvkJvE1DwwU""\n" \
"64+GdsThgvHT7hTkYsXuToy7Jf19GIMAcYeT8DcoYRTrqUWqect1Cz3fu+HkYcJq""\n" \
"amqKqC+Ux2k9mi+QtDqZtPNuj1ZrE9wsMQ7RRQIDAQABAoIBAAESrlMv0jZfPV+e""\n" \
"oAjZ72K59GtxGqq2f4wDuJTq4ey+8vEtfiLftRqH81d1KZEVYJYCxSaVGxALCVuf""\n" \
"Bvr9o+PNyX3+guPJ4clAJ+HgAMKcvX2eYVAZOeyojwDin7zHl45UZlqHhDgYdOGY""\n" \
"9g2RGH+xxcDCTRj+OlVVHUcM3tpsMyjcxkZH1BNWfK+DLbplm0OfI5P9yTVs8uf6""\n" \
"Fmbg2ko2pb7Pgwy2TFHQQDmynE67/GAhR57OJ5g9A1lSvjOxIgli33fJR/CTbPp4""\n" \
"6dmiGBFM/gz9KMDNTmIUCAvVLB0a7zBOr5pXrIQkL7naga9l0LRgWFWUi6PuvzjV""\n" \
"odT5R2kCgYEA+LPvslV72yyAEw+VShWmwcmJiqCYckXOH1fun0+7c2R+ysqg4IBV""\n" \
"eq7rNXFiVSNFJnNpJcCWsv8JGyT0wSySVed5pm04DD4DFbyY0P3ma4LHCW+X80jC""\n" \
"/pdU7GH3XgsgP5kSASCKhOeJEtL+rDdWaHVt8NnzusnJZpEaoJKAfi0CgYEA0zdZ""\n" \
"RS4if5CAMOjBg6FDy9tYNBLwUxfnzgSCdJRro+D2w9lZOsW1w6sNELvVajxUjYPY""\n" \
"E4WHlPFwLXQvb9mkStIFHImuXUwYs1dDsNaW3GSYTRtJ/fJHzxoI4LQmxwoCpKpn""\n" \
"ZQ4Sl+7luuKgE1XmTIT7e0fc/NnTScHCj/XKpnkCgYAMlHV0bNSGAps725bul2fG""\n" \
"953IcFUlupgxh64fStYtgRDUyykWEgfA2+Yz3tddfRo7Vq0j5rj4tuPMBrOGre19""\n" \
"AOdrXqzuGZjoWZDVSXKFT6kntghWmwCaBieaiV4M2L052lFJ006OaHPLivas2WO6""\n" \
"hu4Xt76XXSudz4ssQhzTxQKBgCsvrfkPLc6XZR6a+LZJSutrteUv2iOjR6pAEcx0""\n" \
"bG3dmZcB4dS6iCex2cIKJeDK5R7qTkvviYFSvHUOxIRCI+2Ic7MaE6TP4l0JArYt""\n" \
"qjr19o4NfluGJliaIiaPrz4TNeclUG7BKdwW5LIJlGFftu7+Yc0bLHwmraTvETEO""\n" \
"OJlBAoGAUU1p3AZsibn58b+kofUOoVAksyvkpR02BlcrA3spG/WJ29/K3WTLJ83M""\n" \
"Amwr8hFHKcjSvUQzzx8WjFQC/x8SgcWoJiUv65To2YxEDqUi9gRID94FV+joRchg""\n" \
"bjJk+wapWPFpVDOTYD//mGU8r4FGL5nOzR0WCuV0KhQNcTTAuIM=""\n" \
"-----END RSA PRIVATE KEY-----"

#define X509_TEST_RSA_PRIVATE_KEY_PKCS8 \
"-----BEGIN PRIVATE KEY-----""\n" \
"MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQDNMhPyP2vm8GRp""\n" \
"KbhZ869mnfAm/Mgl467KrzT0iioDGe70ZKrX+tZIYIUGlLeJLouxP25DIgkd7yjZ""\n" \
"633IZUG3gU9/g9MnXABuWYjKRietntb0I07Qx7iIJxctj0qcAABwhKZzJtgOSHw2""\n" \
"P7E1yvgPzp2lHIIJFTkBZIko7VCLaFhdMrMz7iMAfLlrAldzek8l0NufCO5HOx/A""\n" \
"FNAiwriytKujpoB0bqmckdDlu+Qm8TUPDBTrj4Z2xOGC8dPuFORixe5OjLsl/X0Y""\n" \
"gwBxh5PwNyhhFOupRap5y3ULPd+74eRhwmpqaoqoL5THaT2aL5C0Opm0826PVmsT""\n" \
"3CwxDtFFAgMBAAECggEAARKuUy/SNl89X56gCNnvYrn0a3EaqrZ/jAO4lOrh7L7y""\n" \
"8S1+It+1GofzV3UpkRVglgLFJpUbEAsJW58G+v2j483Jff6C48nhyUAn4eAAwpy9""\n" \
"fZ5hUBk57KiPAOKfvMeXjlRmWoeEOBh04Zj2DZEYf7HFwMJNGP46VVUdRwze2mwz""\n" \
"KNzGRkfUE1Z8r4MtumWbQ58jk/3JNWzy5/oWZuDaSjalvs+DDLZMUdBAObKcTrv8""\n" \
"YCFHns4nmD0DWVK+M7EiCWLfd8lH8JNs+njp2aIYEUz+DP0owM1OYhQIC9UsHRrv""\n" \
"ME6vmleshCQvudqBr2XQtGBYVZSLo+6/ONWh1PlHaQKBgQD4s++yVXvbLIATD5VK""\n" \
"FabByYmKoJhyRc4fV+6fT7tzZH7KyqDggFV6rus1cWJVI0Umc2klwJay/wkbJPTB""\n" \
"LJJV53mmbTgMPgMVvJjQ/eZrgscJb5fzSML+l1TsYfdeCyA/mRIBIIqE54kS0v6s""\n" \
"N1ZodW3w2fO6yclmkRqgkoB+LQKBgQDTN1lFLiJ/kIAw6MGDoUPL21g0EvBTF+fO""\n" \
"BIJ0lGuj4PbD2Vk6xbXDqw0Qu9VqPFSNg9gThYeU8XAtdC9v2aRK0gUcia5dTBiz""\n" \
"V0Ow1pbcZJhNG0n98kfPGgjgtCbHCgKkqmdlDhKX7uW64qATVeZMhPt7R9z82dNJ""\n" \
"wcKP9cqmeQKBgAyUdXRs1IYCmzvblu6XZ8b3nchwVSW6mDGHrh9K1i2BENTLKRYS""\n" \
"B8Db5jPe1119GjtWrSPmuPi248wGs4at7X0A52terO4ZmOhZkNVJcoVPqSe2CFab""\n" \
"AJoGJ5qJXgzYvTnaUUnTTo5oc8uK9qzZY7qG7he3vpddK53PiyxCHNPFAoGAKy+t""\n" \
"+Q8tzpdlHpr4tklK62u15S/aI6NHqkARzHRsbd2ZlwHh1LqIJ7HZwgol4MrlHupO""\n" \
"S++JgVK8dQ7EhEIj7YhzsxoTpM/iXQkCti2qOvX2jg1+W4YmWJoiJo+vPhM15yVQ""\n" \
"bsEp3BbksgmUYV+27v5hzRssfCatpO8RMQ44mUECgYBRTWncBmyJufnxv6Sh9Q6h""\n" \
"UCSzK+SlHTYGVysDeykb9Ynb38rdZMsnzcwCbCvyEUcpyNK9RDPPHxaMVAL/HxKB""\n" \
"xagmJS/rlOjZjEQOpSL2BEgP3gVX6OhFyGBuMmT7BqlY8WlUM5NgP/+YZTyvgUYv""\n" \
"mc7NHRYK5XQqFA1xNMC4gw==""\n" \
"-----END PRIVATE KEY-----"

#define X509_TEST_RSA_PRIVATE_KEY_PKCS8_ENCRYPTED \
"-----BEGIN ENCRYPTED PRIVATE KEY-----""\n" \
"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""\n" \
"-----END ENCRYPTED PRIVATE KEY-----"

#define X509_TEST_ECC_CERTIFICATE \
"-----BEGIN CERTIFICATE-----""\n" \
"MIIBtjCCAV2gAwIBAgIUTIyexc2W9eUMD9na6dpAaEXWu/swCgYIKoZIzj0EAwIw""\n" \
"MDEuMCwGA1UEAwwlYXp1cmUtYy1zaGFyZWQtdXRpbGl0eSB4NTA5IHRlc3QgKEVD""\n" \
"KTAgFw0yNjEwMDYwMzM1MTBaGA8yMTI2MDkxMjAzMzUxMFowMDEuMCwGA1UEAwwl""\n" \
"YXp1cmUtYy1zaGFyZWQtdXRpbGl0eSB4NTA5IHRlc3QgKEVDKTBZMBMGByqGSM49""\n" \
"AgEGCCqGSM49AwEHA0IABLlRU4/G4hBTUQirab9Z+2wbpBnD/QUcRlTERJxbo07f""\n" \
"Z7y6+rmQ4nBq7YyCz5pKp7Bos/fqa1bUoBvn3AZz0u+jUzBRMB0GA1UdDgQWBBQV""\n" \
"XDNbCTv6de7fSdbBSN268mNVTDAfBgNVHSMEGDAWgBQVXDNbCTv6de7fSdbBSN26""\n" \
"8mNVTDAPBgNVHRMBAf8EBTADAQH/MAoGCCqGSM49BAMCA0cAMEQCIAjBor4gE2TR""\n" \
"UcNjT4oXSvFwExQQBiR1PpWR300fupc/AiAZX3sVKXUZ5UiAMce90zr1b+PXN3Hz""\n" \
"sgHlzVz01iy9jg==""\n" \
"-----END CERTIFICATE-----"

#define X509_TEST_ECC_PRIVATE_KEY_SEC1 \
"-----BEGIN EC PRIVATE KEY-----""\n" \
"MHcCAQEEIDnns6AXyc/YwHcBu1hriaIP2PoUJf540scsBOAnTopOoAoGCCqGSM49""\n" \
"AwEHoUQDQgAEuVFTj8biEFNRCKtpv1n7bBukGcP9BRxGVMREnFujTt9nvLr6uZDi""\n" \
"cGrtjILPmkqnsGiz9+prVtSgG+fcBnPS7w==""\n" \
"-----END EC PRIVATE KEY-----"

#define X509_TEST_ECC_PRIVATE_KEY_PKCS8 \
"-----BEGIN PRIVATE KEY-----""\n" \
"MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgOeezoBfJz9jAdwG7""\n" \
"WGuJog/Y+hQl/njSxywE4CdOik6hRANCAAS5UVOPxuIQU1EIq2m/WftsG6QZw/0F""\n" \
"HEZUxEScW6NO32e8uvq5kOJwau2Mgs+aSqewaLP36mtW1KAb59wGc9Lv""\n" \
"-----END PRIVATE KEY-----"


// Tests whether serverCertificate chains up to trustedCertificate or not
static void test_VerifyCertificateChain(const char* trustedCertificate, const char* serverCertificate, bool expectSuccess)
{
    char serverCertificateEncoded[TEST_MAX_SERVER_CERTIFICATE_ENCODE_SIZE];
    DWORD serverCertificateEncodedLen = sizeof(serverCertificateEncoded);

    ASSERT_IS_TRUE(CryptStringToBinaryA(serverCertificate, 0, CRYPT_STRING_ANY, (BYTE*)serverCertificateEncoded, &serverCertificateEncodedLen, NULL, NULL)==TRUE, "CryptStringToBinaryA fails, GetLastError=0x%08x", GetLastError());

    PCCERT_CONTEXT pCertContext = CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, (const BYTE*)serverCertificateEncoded, serverCertificateEncodedLen);
    ASSERT_IS_NOT_NULL(pCertContext, "CertCreateCertificateContext fails, GetLastError=0x%08x", GetLastError());

    int result = x509_verify_certificate_in_chain(trustedCertificate, pCertContext);

    if (expectSuccess)
    {
        ASSERT_ARE_EQUAL(int, result, 0, "x509_verify_certificate_in_chain failed but should've succeeded");
    }
    else
    {
        ASSERT_ARE_NOT_EQUAL(int, result, 0, "x509_verify_certificate_in_chain succeeded but should've failed");

    }
}

// Deletes the CNG key x509_schannel_create persisted, so the test leaves no key behind.
static void test_DeletePersistedKey(const wchar_t* containerName)
{
    NCRYPT_PROV_HANDLE provider = 0;

    if ((containerName[0] != L'\0') && (NCryptOpenStorageProvider(&provider, MS_KEY_STORAGE_PROVIDER, 0) == ERROR_SUCCESS))
    {
        NCRYPT_KEY_HANDLE key = 0;

        if (NCryptOpenKey(provider, &key, containerName, 0, 0) == ERROR_SUCCESS)
        {
            // NCryptDeleteKey frees the key handle as well.
            (void)NCryptDeleteKey(key, 0);
        }
        (void)NCryptFreeObject(provider);
    }
}

// Copies the name of the key container x509_schannel_create bound to the certificate. The
// property belongs to the certificate context, so it has to be taken before the handle is freed.
static void test_GetKeyContainerName(PCCERT_CONTEXT certificateContext, wchar_t* containerName, size_t containerNameLength)
{
    CRYPT_KEY_PROV_INFO* provInfo = NULL;
    DWORD provInfoSize = 0;

    containerName[0] = L'\0';

    if (CertGetCertificateContextProperty(certificateContext, CERT_KEY_PROV_INFO_PROP_ID, NULL, &provInfoSize) &&
        ((provInfo = (CRYPT_KEY_PROV_INFO*)malloc(provInfoSize)) != NULL) &&
        CertGetCertificateContextProperty(certificateContext, CERT_KEY_PROV_INFO_PROP_ID, provInfo, &provInfoSize) &&
        (provInfo->pwszContainerName != NULL))
    {
        (void)wcscpy_s(containerName, containerNameLength, provInfo->pwszContainerName);
    }
    free(provInfo);
}

// Drives a certificate and one encoding of its private key through x509_schannel_create, then
// signs with the imported key and verifies the signature with the public key in the certificate.
// That covers the whole decode -> import -> bind path and proves the key the adapter installed is
// usable and really is the one belonging to the certificate.
static void test_PrivateKeySignsForCertificate(const char* certificate, const char* privateKey, bool isEcc)
{
    // Signed as-is; the value only has to be the digest length the algorithm expects.
    static const BYTE digest[32] =
    {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
    };
    BCRYPT_PKCS1_PADDING_INFO padding = { BCRYPT_SHA256_ALGORITHM };
    void* signPadding = isEcc ? NULL : &padding;
    DWORD signFlags = isEcc ? 0 : BCRYPT_PAD_PKCS1;
    HCRYPTPROV_OR_NCRYPT_KEY_HANDLE privateKeyHandle = 0;
    DWORD keySpec = 0;
    BOOL callerFreeKey = FALSE;
    BCRYPT_KEY_HANDLE publicKeyHandle = NULL;
    BYTE signature[512];
    DWORD signatureLength = 0;
    SECURITY_STATUS status;
    wchar_t containerName[256];
    PCCERT_CONTEXT certificateContext;

    X509_SCHANNEL_HANDLE handle = x509_schannel_create(certificate, privateKey);
    ASSERT_IS_NOT_NULL(handle, "x509_schannel_create failed, GetLastError=0x%08x", GetLastError());

    certificateContext = x509_schannel_get_certificate_context(handle);
    ASSERT_IS_NOT_NULL(certificateContext, "x509_schannel_get_certificate_context returned NULL");

    test_GetKeyContainerName(certificateContext, containerName, sizeof(containerName) / sizeof(containerName[0]));

    ASSERT_IS_TRUE(CryptAcquireCertificatePrivateKey(certificateContext, CRYPT_ACQUIRE_ONLY_NCRYPT_KEY_FLAG | CRYPT_ACQUIRE_SILENT_FLAG, NULL, &privateKeyHandle, &keySpec, &callerFreeKey) == TRUE,
        "the private key is not usable through the certificate, GetLastError=0x%08x", GetLastError());
    ASSERT_IS_TRUE(keySpec == CERT_NCRYPT_KEY_SPEC, "the certificate is not bound to a CNG key, keySpec=0x%08x", (unsigned int)keySpec);

    status = NCryptSignHash((NCRYPT_KEY_HANDLE)privateKeyHandle, signPadding, (PBYTE)digest, sizeof(digest), signature, sizeof(signature), &signatureLength, signFlags);
    ASSERT_ARE_EQUAL(int, 0, (int)status, "NCryptSignHash failed with 0x%08x", (unsigned int)status);

    ASSERT_IS_TRUE(CryptImportPublicKeyInfoEx2(X509_ASN_ENCODING, &certificateContext->pCertInfo->SubjectPublicKeyInfo, 0, NULL, &publicKeyHandle) == TRUE,
        "could not import the public key of the certificate, GetLastError=0x%08x", GetLastError());

    status = (SECURITY_STATUS)BCryptVerifySignature(publicKeyHandle, signPadding, (PUCHAR)digest, sizeof(digest), signature, signatureLength, signFlags);
    ASSERT_ARE_EQUAL(int, 0, (int)status, "the signature does not verify against the certificate, status=0x%08x", (unsigned int)status);

    (void)BCryptDestroyKey(publicKeyHandle);
    if (callerFreeKey)
    {
        (void)NCryptFreeObject((NCRYPT_KEY_HANDLE)privateKeyHandle);
    }
    x509_schannel_destroy(handle);
    test_DeletePersistedKey(containerName);
}

BEGIN_TEST_SUITE(x509_schannel_int)

TEST_SUITE_INITIALIZE(suite_init)
{
    ;
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    ;
}

TEST_FUNCTION_INITIALIZE(function_init)
{
    ;
}

TEST_FUNCTION_CLEANUP(function_cleanup)
{
    ;
}


// Test server certificate chains directly to the trusted root
TEST_FUNCTION(x509_verify_certificate_chains_one_trusted_cert)
{
    test_VerifyCertificateChain(X509_TEST_CERTIFICATE_CHAIN1, x509_test_server_certificate, true);
}

// Test that server certificate that does not chain is rejected
TEST_FUNCTION(x509_verify_certificate_does_not_chain_one_trusted_cert)
{
    test_VerifyCertificateChain(X509_TEST_CERTIFICATE_CHAIN2, x509_test_server_certificate, false);
}

// Test server certificate chains directly to the trusted root and 2 certs are passed
TEST_FUNCTION(x509_verify_certificate_chains_two_trusted_certs)
{
    test_VerifyCertificateChain(X509_TEST_CERTIFICATE_CHAIN1 X509_TEST_CERTIFICATE_CHAIN2, x509_test_server_certificate, true);
}

// Test that when 3 trusted certs are passed in, and the root we chain to comes first, that we succeed.
TEST_FUNCTION(x509_verify_certificate_chains_three_certs_root_first)
{
    test_VerifyCertificateChain(X509_TEST_CERTIFICATE_CHAIN1 X509_TEST_CERTIFICATE_CHAIN2 X509_TEST_CERTIFICATE_CHAIN3, x509_test_server_certificate, true);
}

// Test that when 3 trusted certs are passed in, and the root we chain to comes in the middle, that we succeed.
TEST_FUNCTION(x509_verify_certificate_chains_three_certs_root_middle)
{
    test_VerifyCertificateChain(X509_TEST_CERTIFICATE_CHAIN2 X509_TEST_CERTIFICATE_CHAIN1 X509_TEST_CERTIFICATE_CHAIN3, x509_test_server_certificate, true);
}

// Test that when 2 trusted certs are passed in but we don't chain to any of them, that we fail.
TEST_FUNCTION(x509_verify_certificate_no_chains_two_certs)
{
    test_VerifyCertificateChain(X509_TEST_CERTIFICATE_CHAIN2 X509_TEST_CERTIFICATE_CHAIN3, x509_test_server_certificate, false);
}

// Test that when 3 trusted certs are passed in but we don't chain to any of them, that we fail.
TEST_FUNCTION(x509_verify_certificate_no_chains_three_certs)
{
    test_VerifyCertificateChain(X509_TEST_CERTIFICATE_CHAIN2 X509_TEST_CERTIFICATE_CHAIN3 X509_TEST_CERTIFICATE_CHAIN4, x509_test_server_certificate, false);
}

// A PKCS#1 RSA private key is bound to its certificate and can sign with it.
TEST_FUNCTION(x509_schannel_rsa_pkcs1_private_key_signs_for_its_certificate)
{
    test_PrivateKeySignsForCertificate(X509_TEST_RSA_CERTIFICATE, X509_TEST_RSA_PRIVATE_KEY_PKCS1, false);
}

// The same RSA key in a PKCS#8 PrivateKeyInfo is unwrapped and works identically.
TEST_FUNCTION(x509_schannel_rsa_pkcs8_private_key_signs_for_its_certificate)
{
    test_PrivateKeySignsForCertificate(X509_TEST_RSA_CERTIFICATE, X509_TEST_RSA_PRIVATE_KEY_PKCS8, false);
}

// An RFC 5915 ECC private key is bound to its certificate and can sign with it.
TEST_FUNCTION(x509_schannel_ecc_sec1_private_key_signs_for_its_certificate)
{
    test_PrivateKeySignsForCertificate(X509_TEST_ECC_CERTIFICATE, X509_TEST_ECC_PRIVATE_KEY_SEC1, true);
}

// The same ECC key in a PKCS#8 PrivateKeyInfo, whose inner ECPrivateKey carries no curve OID.
TEST_FUNCTION(x509_schannel_ecc_pkcs8_private_key_signs_for_its_certificate)
{
    test_PrivateKeySignsForCertificate(X509_TEST_ECC_CERTIFICATE, X509_TEST_ECC_PRIVATE_KEY_PKCS8, true);
}

// Encrypted PKCS#8 is not supported and must be rejected rather than half-imported.
TEST_FUNCTION(x509_schannel_encrypted_pkcs8_private_key_is_rejected)
{
    X509_SCHANNEL_HANDLE handle = x509_schannel_create(X509_TEST_RSA_CERTIFICATE, X509_TEST_RSA_PRIVATE_KEY_PKCS8_ENCRYPTED);
    ASSERT_IS_NULL(handle, "an encrypted PKCS#8 private key was accepted");
}

END_TEST_SUITE(x509_schannel_int)

#endif /*WIN32*/

