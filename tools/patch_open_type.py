#!/usr/bin/env python3
import os
import glob

def patch_file(filepath):
    with open(filepath, 'r') as f:
        content = f.read()

    original_content = content
    filename = os.path.basename(filepath)

    if filename in ["OPEN_TYPE_aper.c", "OPEN_TYPE_oer.c", "OPEN_TYPE_uper.c", "OPEN_TYPE_xer.c"]:
        # 1. Inject is_choice and is_ioc_inline logic where `elements_count > 0` is checked in the get/decode function
        replacement_1 = """    int is_choice = (elm->type->elements_count > 0);
    int is_ioc_inline = (elm->type->elements_count == 0 && elm->type_selector != NULL);
    /* Check if this OPEN_TYPE uses CHOICE wrapper (elements_count > 0) or direct type */
    if(is_choice || is_ioc_inline) {"""
        content = content.replace("    /* Check if this OPEN_TYPE uses CHOICE wrapper (elements_count > 0) or direct type */\n    if(elm->type->elements_count > 0) {", replacement_1)

        # 1b. Fix the bounds check
        search_bounds = """        /* Validate the selected variant */
        if(selected.presence_index > elm->type->elements_count) {"""
        replace_bounds = """        /* Validate the selected variant */
        if(is_choice && selected.presence_index > elm->type->elements_count) {"""
        content = content.replace(search_bounds, replace_bounds)

        # 1c. Fix elements array check
        search_elements = """        /* Ensure we can access the elements array if needed */
        if(!elm->type->elements) {"""
        replace_elements = """        /* Ensure we can access the elements array if needed */
        if(is_choice && !elm->type->elements) {"""
        content = content.replace(search_elements, replace_elements)

        # 1d. Fix CHOICE allocation
        search_alloc = """        /* Allocate the CHOICE structure if not already present */
        if(*memb_ptr2 == NULL) {
            const asn_CHOICE_specifics_t *specs = 
                (const asn_CHOICE_specifics_t *)elm->type->specifics;
            if(!specs) {
                ASN_DEBUG("Open Type %s->%s: type specifics is NULL",
                          td->name, elm->name);
                ASN__DECODE_FAILED;
            }
            *memb_ptr2 = CALLOC(1, specs->struct_size);
            if(*memb_ptr2 == NULL) {
                ASN__DECODE_FAILED;
            }"""
        replace_alloc = """        /* Allocate the CHOICE structure if not already present */
        if(*memb_ptr2 == NULL) {
            if (is_ioc_inline) {
                /* We don't have specs for IOC inline CHOICE, so just allocate a conservative size, or assume the parent already allocated it!
                 * Wait, for IOC inline, *memb_ptr2 is ALWAYS a pointer to the embedded struct, so it's NEVER NULL!
                 * But if it was a pointer, we wouldn't have the size. Just return failure.
                 */
                ASN__DECODE_FAILED;
            } else {
                const asn_CHOICE_specifics_t *specs = 
                    (const asn_CHOICE_specifics_t *)elm->type->specifics;
                if(!specs) {
                    ASN_DEBUG("Open Type %s->%s: type specifics is NULL",
                              td->name, elm->name);
                    ASN__DECODE_FAILED;
                }
                *memb_ptr2 = CALLOC(1, specs->struct_size);
                if(*memb_ptr2 == NULL) {
                    ASN__DECODE_FAILED;
                }
            }"""
        content = content.replace(search_alloc, replace_alloc)

        # 1e. Fix the CHOICE presence reset
        search_reset = """        } else {
            /* Make sure we reset the structure first before decoding */
            if(CHOICE_variant_set_presence(elm->type, *memb_ptr2, 0)"""
        replace_reset = """        } else {
            /* Make sure we reset the structure first before decoding */
            if(is_choice && CHOICE_variant_set_presence(elm->type, *memb_ptr2, 0)"""
        content = content.replace(search_reset, replace_reset)

        # 2. Fix the inner block where it gets variant info and offset
        search_2 = """        if(elm->type->elements && selected.presence_index > 0 
           && selected.presence_index <= elm->type->elements_count) {"""
        replace_2 = """        if(is_choice && elm->type->elements && selected.presence_index > 0 
           && selected.presence_index <= elm->type->elements_count) {"""
        content = content.replace(search_2, replace_2)

        # 3. Handle the offset for is_ioc_inline
        search_3 = """        } else {
            /* Compute address of the embedded value field */
            inner_value = (char *)*memb_ptr2 + memb_offset;
        }"""
        replace_3 = """        } else {
            /* Compute address of the embedded value field */
            if (is_ioc_inline) {
                memb_offset = (sizeof(void*) == 8) ? 8 : 4;
            }
            inner_value = (char *)*memb_ptr2 + memb_offset;
        }"""
        content = content.replace(search_3, replace_3)

        # 4. Handle CHOICE_variant_set_presence success check block
        search_4 = """    case RC_OK:
        if(elm->type->elements_count > 0) {"""
        replace_4 = """    case RC_OK:
        if(is_choice || is_ioc_inline) {"""
        content = content.replace(search_4, replace_4)

        # 5. Fix CHOICE_variant_set_presence calling
        search_5 = """            if(CHOICE_variant_set_presence(elm->type, *memb_ptr2,
                                           selected.presence_index)
               == 0) {"""
        replace_5 = """            int presence_set = 0;
            if (is_ioc_inline) {
                unsigned *present_ptr = (unsigned *)*memb_ptr2;
                *present_ptr = selected.presence_index;
                presence_set = 1;
            } else {
                presence_set = (CHOICE_variant_set_presence(elm->type, *memb_ptr2, selected.presence_index) == 0);
            }
            if(presence_set) {"""
        content = content.replace(search_5, replace_5)

        # 5b. Fix CHOICE_variant_set_presence calling for XER which uses != 0
        search_5b = """            if(CHOICE_variant_set_presence(elm->type, *memb_ptr2,
                                           selected.presence_index)
               != 0) {"""
        replace_5b = """            int presence_set = 0;
            if (is_ioc_inline) {
                unsigned *present_ptr = (unsigned *)*memb_ptr2;
                *present_ptr = selected.presence_index;
                presence_set = 1;
            } else {
                presence_set = (CHOICE_variant_set_presence(elm->type, *memb_ptr2, selected.presence_index) == 0);
            }
            if(!presence_set) {"""
        content = content.replace(search_5b, replace_5b)

        # 6. Fix cleanup block
        search_6 = """    case RC_WMORE:
    case RC_FAIL:
        ASN_DEBUG("Cleaning up after failure, code=%d", rv.code);
        if(elm->type->elements_count > 0) {"""
        replace_6 = """    case RC_WMORE:
    case RC_FAIL:
        ASN_DEBUG("Cleaning up after failure, code=%d", rv.code);
        if(is_choice || is_ioc_inline) {"""
        content = content.replace(search_6, replace_6)
        
        search_6b = """        if(elm->type->elements_count > 0) {
            /*
             * CHOICE wrapper mode.  For indirect CHOICE variants,"""
        replace_6b = """        if(is_choice || is_ioc_inline) {
            /*
             * CHOICE wrapper mode.  For indirect CHOICE variants,"""
        content = content.replace(search_6b, replace_6b)

        # 7. Fix the `_put` or encode functions logic
        search_7 = """    if(elm->type->elements_count > 0) {
        /* 
         * CHOICE wrapper mode: encode the CHOICE directly."""
        replace_7 = """    int is_choice = (elm->type->elements_count > 0);
    int is_ioc_inline = (elm->type->elements_count == 0 && elm->type_selector != NULL);
    if(is_choice) {
        /* 
         * CHOICE wrapper mode: encode the CHOICE directly."""
        content = content.replace(search_7, replace_7)

        search_7_xer = """    if(elm->type->elements_count > 0) {
        /* CHOICE wrapper mode: use standard CHOICE encoder */"""
        replace_7_xer = """    int is_choice = (elm->type->elements_count > 0);
    int is_ioc_inline = (elm->type->elements_count == 0 && elm->type_selector != NULL);
    if(is_choice) {
        /* CHOICE wrapper mode: use standard CHOICE encoder */"""
        content = content.replace(search_7_xer, replace_7_xer)

        # 8. Fix the direct type mode inside `_put` to handle offset
        search_8 = """    } else {
        /* 
         * Direct type mode: encode using the selected type descriptor."""
        replace_8 = """    } else {
        /* 
         * Direct type mode: encode using the selected type descriptor.
         */
        if (is_ioc_inline) {
            unsigned int choice_offset = (sizeof(void*) == 8) ? 8 : 4;
            memb_ptr = (const void *)((const char *)memb_ptr + choice_offset);
        }
        /*"""
        content = content.replace(search_8, replace_8)
        
        search_8_xer = """    } else {
        /* Direct type mode: encode using the selected type descriptor with wrapper tag */"""
        replace_8_xer = """    } else {
        /* Direct type mode: encode using the selected type descriptor with wrapper tag */
        if (is_ioc_inline) {
            unsigned int choice_offset = (sizeof(void*) == 8) ? 8 : 4;
            memb_ptr = (const void *)((const char *)memb_ptr + choice_offset);
        }"""
        content = content.replace(search_8_xer, replace_8_xer)

    if content != original_content:
        with open(filepath, 'w') as f:
            f.write(content)
        print(f"Patched {filepath}")
        return True
    return False

def main():
    # First revert from patched files!
    os.system("cp ~/airspan/asn-codec/OPEN_TYPE_aper.c.patched ~/airspan/asn-codec/generated/ngap/OPEN_TYPE_aper.c")
    os.system("cp ~/airspan/asn-codec/OPEN_TYPE_oer.c.patched ~/airspan/asn-codec/generated/ngap/OPEN_TYPE_oer.c")
    os.system("cp ~/airspan/asn-codec/OPEN_TYPE_uper.c.patched ~/airspan/asn-codec/generated/ngap/OPEN_TYPE_uper.c")
    os.system("cp ~/airspan/asn-codec/OPEN_TYPE_xer.c.patched ~/airspan/asn-codec/generated/ngap/OPEN_TYPE_xer.c")
    # Actually, better to fetch from git to be safe, but wait, the repo has generated/ committed!
    os.system("git -C ~/airspan/asn-codec checkout -- generated/")
    
    base_dir = os.path.expanduser("~/airspan/asn-codec/generated")
    patched_count = 0
    for root, dirs, files in os.walk(base_dir):
        for f in files:
            if f in ["OPEN_TYPE_aper.c", "OPEN_TYPE_oer.c", "OPEN_TYPE_uper.c", "OPEN_TYPE_xer.c"]:
                if patch_file(os.path.join(root, f)):
                    patched_count += 1
    
    print(f"Total files patched: {patched_count}")

if __name__ == "__main__":
    main()
