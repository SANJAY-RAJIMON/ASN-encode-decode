import sys
import re

if len(sys.argv) < 2:
    print("Usage: python3 fix_duplicates.py <file>")
    sys.exit(1)

filename = sys.argv[1]

with open(filename, 'r') as f:
    content = f.read()

def remove_duplicate_funcs(name):
    global content
    pattern = r'^static int\s*\n*' + re.escape(name) + r'\s*\([^{]*\{[^}]*\}\n*'
    matches = list(re.finditer(pattern, content, re.MULTILINE))
    if len(matches) > 1:
        for m in reversed(matches[1:]):
            content = content[:m.start()] + content[m.end():]

remove_duplicate_funcs('memb_id_constraint_0')
remove_duplicate_funcs('memb_criticality_constraint_0')
remove_duplicate_funcs('memb_value_constraint_0')

def remove_duplicate_def(name):
    global content
    pattern = r'^(?:static\s+)?(?:const\s+)?\w+(?:\s+\w+)?\s+' + re.escape(name) + r'(?:\[\])?(?: CC_NOTUSED)?\s*=\s*\{'
    matches = list(re.finditer(pattern, content, re.MULTILINE))
    if len(matches) > 1:
        for m in reversed(matches[1:]):
            start = m.start()
            brace_count = 0
            end = start
            found_brace = False
            for i in range(m.end() - 1, len(content)):
                if content[i] == '{':
                    brace_count += 1
                    found_brace = True
                elif content[i] == '}':
                    brace_count -= 1
                if found_brace and brace_count == 0:
                    end = i + 1
                    break
            while end < len(content) and content[end] in [';', ' ', '\t', '\n', '\r']:
                end += 1
            content = content[:start] + content[end:]

remove_duplicate_def('asn_OER_memb_criticality_constr_0')
remove_duplicate_def('asn_PER_memb_criticality_constr_0')
remove_duplicate_def('asn_OER_memb_id_constr_0')
remove_duplicate_def('asn_PER_memb_id_constr_0')
remove_duplicate_def('asn_OER_memb_value_constr_0')
remove_duplicate_def('asn_PER_memb_value_constr_0')

remove_duplicate_def('asn_SPC_value_specs_0')
remove_duplicate_def('asn_DEF_value_0')
remove_duplicate_def('asn_MAP_value_tag2el_0')

# Inject missing definitions at the top
missing_defs = """
#if !defined(ASN_DISABLE_OER_SUPPORT)
static asn_oer_constraints_t asn_OER_memb_id_constr_0 CC_NOTUSED = { { 0, 0 }, -1 };
static asn_oer_constraints_t asn_OER_memb_value_constr_0 CC_NOTUSED = { { 0, 0 }, -1 };
#endif  /* !defined(ASN_DISABLE_OER_SUPPORT) */
#if !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT)
static asn_per_constraints_t asn_PER_memb_id_constr_0 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 16,  16,  0,  65535 }	/* (0..65535) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_per_constraints_t asn_PER_memb_value_constr_0 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
#endif  /* !defined(ASN_DISABLE_UPER_SUPPORT) || !defined(ASN_DISABLE_APER_SUPPORT) */
"""

if "asn_OER_memb_id_constr_0 CC_NOTUSED =" not in content:
    content = content.replace('#include "ProtocolIE-Field.h"\n', '#include "ProtocolIE-Field.h"\n' + missing_defs)

with open(filename, 'w') as f:
    f.write(content)
