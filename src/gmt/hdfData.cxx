/*\
 * hdfData.cxx
 *
 * Copyright (c) 2015 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/

#include <stdio.h>
#include <string.h> // for strdup(), ...
#include <stdint.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>
#ifdef WIN32

#else
#include <inttypes.h>
#endif
#ifdef  __cplusplus
extern "C" {
#endif
#include <hdf5.h>
// #include <h5dump.h"
//#include "H5private.h"
#include <h5tools.h>
#include <h5tools_utils.h>
#include <h5tools_ref.h>
#include <h5trav.h>
//#include "h5dump_defines.h"
#ifdef  __cplusplus
}
#endif

#include "sprtf.hxx"
#include "utils.hxx"
#include "hdfData.hxx"

/*\
 * from : http://www.hdfgroup.org/HDF5/doc/H5.format.html
 *
\*/

#ifdef WIN32
#define UI64 "I64u"
#define UX64 "I64x"
#else
#define UI64 PRIu64
#define UX64 PRIx64
#endif

static const char *module = "hdfData";
static const char *def_log = "temphdf.txt";

#define DEBUG_ON
static const char *def_input = "D:\\gshhg\\gshhg-gmt-2.3.4\\binned_border_c.nc";
static const char *usr_input = 0;

static unsigned char hdfsig[] = { 0x89, 0x48, 0x44, 0x46, 0x0d, 0x0a, 0x1a, 0x0a };


void give_help( char *name )
{
    SPRTF("%s: usage: [options] usr_input\n", module);
    SPRTF("Options:\n");
    SPRTF(" --help  (-h or -?) = This help and exit(2)\n");
    // TODO: More help
}

int parse_args( int argc, char **argv )
{
    int i,i2,c;
    char *arg, *sarg;
    for (i = 1; i < argc; i++) {
        arg = argv[i];
        i2 = i + 1;
        if (*arg == '-') {
            sarg = &arg[1];
            while (*sarg == '-')
                sarg++;
            c = *sarg;
            switch (c) {
            case 'h':
            case '?':
                give_help(argv[0]);
                return 2;
                break;
            // TODO: Other arguments
            default:
                SPRTF("%s: Unknown argument '%s'. Tyr -? for help...\n", module, arg);
                return 1;
            }
        } else {
            // bear argument
            if (usr_input) {
                SPRTF("%s: Already have input '%s'! What is this '%s'?\n", module, usr_input, arg );
                return 1;
            }
            usr_input = strdup(arg);
        }
    }
#ifdef DEBUG_ON
    if (!usr_input) {
        usr_input = strdup(def_input);
    }
#endif
    if (!usr_input) {
        SPRTF("%s: No user input found in command!\n", module);
        return 1;
    }
    return 0;
}

#pragma pack( push, 1 ) 

// Superblock (Versions 0)
typedef struct tagHDRSB0 {
    unsigned char sig[8]; // Hexadecimal:	89	48	44	46	0d	0a	1a	0a
    char Sbver;     // Version Number of the Superblock - Values of 0, 1 and 2 are defined for this field
    char FSVer;     // Version Number of the File Free-Space Information - The only value currently valid in this field is '0', 
    char RBver;     // Version Number of the Root Group Symbol Table Entry - The only value currently valid in this field is '0'
    char res1;      // 0
    char SHver;     // Version Number of the Shared Header Message Format - The only value currently valid in this field is '0'
    char soof;      // Size of Offsets
    char sool;      // Size of Lengths
    char res2;      // 0
    uint16_t GLNK;  // Group Leaf Node K	- This value must be greater than zero.
    uint16_t GINK;  // Group Internal Node K - This value must be greater than zero.
    uint32_t FCF;   // File Consistency Flags
    uint32_t base;  // Base Address
    uint32_t addr;  // Address of File Free-space Info
    uint32_t end;   // End of File Address
    uint32_t di;    // Driver Information Block Address
    uint32_t st;    // Root Group Symbol Table Entry
} HDRSB0, *PHDRSB0;

// Superblock (Versions 1)
typedef struct tagHDRSB1 {
    unsigned char sig[8]; // Hexadecimal:	89	48	44	46	0d	0a	1a	0a
    char Sbver;     // Version Number of the Superblock - Values of 0, 1 and 2 are defined for this field
    char FSVer;     // Version Number of the File Free-Space Information - The only value currently valid in this field is '0', 
    char RBver;     // Version Number of the Root Group Symbol Table Entry - The only value currently valid in this field is '0'
    char res1;      // 0
    char SHver;     // Version Number of the Shared Header Message Format - The only value currently valid in this field is '0'
    char soof;      // Size of Offsets
    char sool;      // Size of Lengths
    char res2;      // 0
    uint16_t GLNK;  // Group Leaf Node K	- This value must be greater than zero.
    uint16_t GINK;  // Group Internal Node K - This value must be greater than zero.
    uint32_t FCF;   // File Consistency Flags
    uint16_t ISIN;  // Indexed Storage Internal Node K1	
    uint16_t res3;  // Reserved (zero)1
    uint32_t base;  // Base Address
    uint32_t addr;  // Address of File Free-space Info
    uint32_t end;   // End of File Address
    uint32_t di;    // Driver Information Block Address
    uint32_t st;    // Root Group Symbol Table Entry
} HDRSB1, *PHDRSB1;

// Superblock (Version 2)
typedef struct tagDRSB2 {
    unsigned char sig[8]; // Hexadecimal:	89	48	44	46	0d	0a	1a	0a
    char Sbver;     // Version Number of the Superblock - Values of 0, 1 and 2 are defined for this field
    char soof;      // Size of Offsets
    char sool;      // Size of Lengths
    char FCF;       // File Consistency Flags
    uint32_t unk1;  // ????
    uint32_t unk2;  // ????
    uint32_t base;  // Base Address
    uint32_t addr;  // Superblock Extension Address
    uint32_t end;   // End of File Address
    uint32_t di;    // Root Group Object Header Address
    uint32_t unk3;  // ????
    uint32_t unk4;  // ????
    uint32_t st;    // Superblock Checksum
} HDRSB2, *PHDRSB2;

typedef struct tagOBJTIMES {
    uint32_t acces; // Access Time - This field is present if bit 5 of flags is set.
    uint32_t modtm; // Modification Time
    uint32_t change; // Change Time
    uint32_t birth; // Birth Time
} OBJTIMES, *POBJTIMES;

typedef struct tagHMTYPE {
    uint16_t hmtype;
    uint16_t hmsize;
    uint8_t hmflag;
    uint8_t res1[3];
    uint8_t hmdata;
} HMTYPE, *PHMTYPE;

// Version 1 Object Header
typedef struct tagOBJHDR1 {
    uint8_t vers;
    uint8_t res1;
    uint16_t hdrcnt;
    uint32_t orefcnt;
    uint32_t ohdrsize;
    HMTYPE hmt;
    //uint16_t hmtype;
    //uint16_t hmsize;
    //uint8_t hmflag;
    //uint8_t res1[3];
    //uint8_t hmdata;
    // hmt repeated
} OBJHDR1, *POBJHDR1;

// Version 2 Object Header
typedef struct tagOBJHDR {
    uint8_t sig[4];
    uint8_t vers;
    uint8_t flags;
    //OBJTIMES times;

} OBJHDR, *POBJHDR;

#pragma pack( pop )

/* ====================================
hdfData: Opening file 'D:\gshhg\gshhg-gmt-2.3.4\binned_border_c.nc',
 51064 (0xC778) bytes
89 48 44 46 0d 0a 1a 0a  02 08 08 00 00 00 00 00 .HDF............
00 00 00 00 ff ff ff ff  ff ff ff ff 78 c7 00 00 ............x...
00 00 00 00 30 00 00 00  00 00 00 00 be 7b 7c 54 ....0........{|T

4f 48 44 52 02 0d 28 01  02 22 00 00 00 00 00 03 OHDR..(.."......
11 00 00 00 00 00 00 00  47 0f 00 00 00 00 00 00 ........G.......
d9 0f 00 00 00 00 00 00  ff 0f 00 00 00 00 00 00 ................
0a 02 00 01 00 00 00 00  10 10 00 00 00 00 70 02 ..............p.
00 00 00 00 00 00 56 00  00 00 00 00 00 00 0c 4c ......V........L
00 00 00 00 03 00 06 00  08 00 04 00 00 74 69 74 .............tit
6c 65 00 13 00 00 00 31  00 00 00 02 00 00 00 50 le.....1.......P
6f 6c 69 74 69 63 61 6c  20 62 6f 75 6e 64 61 72 olitical boundar
69 65 73 20 64 65 72 69  76 65 64 20 66 72 6f 6d ies derived from
20 43 49 41 20 57 44 42  2d 49 49 20 64 61 74 61  CIA WDB-II data
0c 56 00 00 01 00 03 00  07 00 08 00 04 00 00 73 .V.............s
6f 75 72 63 65 00 13 00  00 00 3a 00 00 00 02 00 ource.....:.....
00 00 50 72 6f 63 65 73  73 65 64 20 62 79 20 50 ..Processed by P
61 75 6c 20 57 65 73 73  65 6c 20 61 6e 64 20 57 aul Wessel and W
61 6c 74 65 72 20 48 2e  20 46 2e 20 53 6d 69 74 alter H. F. Smit
68 2c 20 31 39 39 34 2d  32 30 31 34 0c 22 00 00 h, 1994-2014."..
02 00 03 00 08 00 08 00  04 00 00 76 65 72 73 69 ...........versi
6f 6e 00 13 00 00 00 05  00 00 00 02 00 00 00 32 on.............2
2e 33 2e 34 00 06 00 00  00 00 00 00 00 00 00 00 .3.4............
23 a7 f0 14                                      #...            
            4f 48 44 52  02 0d 00 01 01 14 00 00     OHDR........
00 00 02 01 01 01 01 00  00 00 00 00 00 00 01 00 ................
00 00 00 00 00 00 03 14  00 01 00 00 11 21 1f 00 .............!..
04 00 00 00 00 00 20 00  17 08 00 17 7f 00 00 00 ...... .........
05 02 00 01 00 00 03 0a  08 12 00 00 00 00 03 01 ................
ff ff ff ff ff ff ff ff  04 00 00 00 00 00 00 00 ................
15 1c 00 04 00 00 00 03  08 00 ff ff ff ff ff ff ................
ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff ................
ff ff 0c 2b 00 00 00 00  03 00 06 00 08 00 04 00 ...+............
00 43 4c 41 53 53 00 13  00 00 00 10 00 00 00 02 .CLASS..........
00 00 00 44 49 4d 45 4e  53 49 4f 4e 5f 53 43 41 ...DIMENSION_SCA
4c 45 00 10 10 00 00 00  00 c6 02 00 00 00 00 00 LE..............
00 68 00 00 00 00 00 00  00 10 10 00 00 00 00 d7 .h..0...........
2c 00 00 00 00 00 00 ca  00 00 00 00 00 00 00 00 ,...............
27 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 '...............
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 ................
00 00 00 00 00 00 00 00  00 00 00 00 66 92 3a 48 ............f.:H
4f 43 48 4b 15 1c 00 04  00 00 00 03 03 00 ff ff OCHK............
ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff ................
ff ff ff ff ff ff 00 26  00 00 00 00 00 00 00 00 .......&........
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 ................
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 ................
00 00 66 05 3e f5 4f 43  48 4b 0c 5a 00 00 01 00 ..f.>.OCHK.Z....
03 00 05 00 08 00 04 00  00 4e 41 4d 45 00 13 00 .........NAME...
00 00 40 00 00 00 02 00  00 00 54 68 69 73 20 69 ..@.......This i
73 20 61 20 6e 65 74 43  44 46 20 64 69 6d 65 6e s a netCDF dimen
73 69 6f 6e 20 62 75 74  20 6e 6f 74 20 61 20 6e sion but not a n
65 74 43 44 46 20 76 61  72 69 61 62 6c 65 2e 20 etCDF variable. 
20 20 20 20 20 20 20 20  31 00 76 ea ad 23 4f 48         1.v..#OH
44 52 02 0d 89 01 01 14  00 00 00 00 02 01 01 01 DR..............
a2 00 00 00 00 00 00 00  a2 00 00 00 00 00 00 00 ................
03 14 00 01 00 00 11 21  1f 00 04 00 00 00 00 00 .......!........
20 00 17 08 00 17 7f 00  00 00 05 02 00 01 00 00  ...............
03 0a 08 12 00 00 00 00  03 01 ff ff ff ff ff ff ................
ff ff 88 02 00 00 00 00  00 00 15 1c 00 04 00 00 ................
00 03 04 00 ff ff ff ff  ff ff ff ff ff ff ff ff ................
ff ff ff ff ff ff ff ff  ff ff ff ff 0c 2b 00 00 .............+..
00 00 03 00 06 00 08 00  04 00 00 43 4c 41 53 53 ...........CLASS
00 13 00 00 00 10 00 00  00 02 00 00 00 44 49 4d .............DIM
45 4e 53 49 4f 4e 5f 53  43 41 4c 45 00 0c 5a 00 ENSION_SCALE..Z.
00 01 00 03 00 05 00 08  00 04 00 00 4e 41 4d 45 ............NAME
00 13 00 00 00 40 00 00  00 02 00 00 00 54 68 69 .....@.......Thi
hdfData: Version Number of the Superblock 2 - soof 8, sool 8 - ok
hdfData: End of File Address  51064 0xc778 - ok
   ==================================== */
#define MX_READ 1024
static uint8_t read_buff[MX_READ];

void show_hdr_sizes()
{
    SPRTF("%s HDR0 %ld, HDR1 %ld, HDR2 %ld\n", module,
        sizeof(HDRSB0), sizeof(HDRSB1), sizeof(HDRSB2));
}

int show_gmtime( time_t secs )
{
    char buff[32+2];
    struct tm * ptm;
    ptm = gmtime(&secs);
    if (ptm) {
        char *cp = asctime( ptm );
        if (cp && (strlen(cp) < 32)) {
            strcpy(buff,cp);
            size_t len = strlen(buff);
            while (len) {
                len--;
                if (buff[len] > ' ')
                    break;
                buff[len] = 0;
            }
            SPRTF("%s",buff);
            return 1;
        }
    }
    return 0;
}

void show_flag_8( uint8_t flag )
{
    uint8_t mask = 1;
    int i;
    for (i = 0; i < 8; i++) {
        if (flag & mask)
            SPRTF("1");
        else
            SPRTF("0");
        mask = mask << 1;
    }
}

//#define DATASET         "DS1"
#define DATASET         "*"
#ifndef TRUE
#define TRUE true
#endif

typedef struct tagH5TBLS{
        unsigned long   fileno;         /* File number that these tables refer to */
        hid_t           oid;            /* ID of an object in this file, held open so fileno is consistent */
        table_t         *group_table;   /* Table of groups */
        table_t         *dset_table;    /* Table of datasets */
        table_t         *type_table;    /* Table of datatypes */
} H5TBLS, *PH5TBLS;

/* List of table structures.  There is one table structure for each file */
typedef struct h5dump_table_list_t {
    size_t      nalloc;
    size_t      nused;
    //struct {
    //    unsigned long   fileno;         /* File number that these tables refer to */
    //    hid_t           oid;            /* ID of an object in this file, held open so fileno is consistent */
    //    table_t         *group_table;   /* Table of groups */
    //    table_t         *dset_table;    /* Table of datasets */
    //    table_t         *type_table;    /* Table of datatypes */
    //} *tables;
    PH5TBLS tables;
} h5dump_table_list_t;

h5dump_table_list_t  table_list = {0, 0, NULL};
table_t *group_table = NULL, *dset_table = NULL, *type_table = NULL;
int          unamedtype = 0;     /* shared datatype with no name */

#ifndef HDrealloc
    #define HDrealloc(M,Z)    realloc(M,Z)
#endif /* HDrealloc */
#ifndef HDstrcmp
#define HDstrcmp strcmp
#endif
#ifndef HDstrlen
#define HDstrlen strlen
#endif
#define HDmemset(X,C,Z)     memset((void*)(X),C,Z)

#undef MAX
#define MAX(a,b)    (((a)>(b)) ? (a) : (b))

static void
dump_table(char* tablename, table_t *table)
{
    unsigned u;

    //PRINTSTREAM(rawoutstream,"%s: # of entries = %d\n", tablename,table->nobjs);
    SPRTF("%s: # of entries = %d\n", tablename,table->nobjs);
    for (u = 0; u < table->nobjs; u++) {
        SPRTF("%a %s %d %d\n", table->objs[u].objno,
           table->objs[u].objname,
           table->objs[u].displayed, table->objs[u].recorded);
    }
}

void
dump_tables(find_objs_t *info)
{
    dump_table("group_table", info->group_table);
    dump_table("dset_table", info->dset_table);
    dump_table("type_table", info->type_table);
}

ssize_t
table_list_add(hid_t oid, unsigned long file_no)
{
    size_t      idx;         /* Index of table to use */
    find_objs_t info;

    /* Allocate space if necessary */
    if(table_list.nused == table_list.nalloc) {
        void        *tmp_ptr;

        table_list.nalloc = MAX(1, table_list.nalloc * 2);
        if(NULL == (tmp_ptr = HDrealloc(table_list.tables, table_list.nalloc * sizeof(table_list.tables[0]))))
            return -1;
        table_list.tables = (PH5TBLS)tmp_ptr;
    } /* end if */

    /* Append it */
    idx = table_list.nused++;
    table_list.tables[idx].fileno = file_no;
    table_list.tables[idx].oid = oid;
    if(H5Iinc_ref(oid) < 0) {
        table_list.nused--;
        return -1;
    }
    if(init_objs(oid, &info, &table_list.tables[idx].group_table,
                 &table_list.tables[idx].dset_table, &table_list.tables[idx].type_table) < 0) {
        H5Idec_ref(oid);
        table_list.nused--;
        return -1;
    }

//#ifdef H5DUMP_DEBUG
    dump_tables(&info);
//#endif /* H5DUMP_DEBUG */

    return((ssize_t) idx);
} /* end table_list_add() */

/* the table of dump functions */
typedef struct dump_functions_t {
    void     (*dump_group_function) (hid_t, const char *);
    void     (*dump_named_datatype_function) (hid_t, const char *);
    void     (*dump_dataset_function) (hid_t, const char *, struct subset_t *);
    void     (*dump_dataspace_function) (hid_t);
    void     (*dump_datatype_function) (hid_t);
    herr_t   (*dump_attribute_function) (hid_t, const char *, const H5A_info_t *, void *);
    void     (*dump_data_function) (hid_t, int, struct subset_t *, int);
} dump_functions;

const dump_functions *dump_function_table = 0;


void
dump_group(hid_t gid, const char *name)
{
    H5O_info_t  oinfo;
    hid_t       dset;
    hid_t       type;
    hid_t       gcpl_id;
    unsigned    crt_order_flags;
    unsigned    attr_crt_order_flags;
    char        type_name[1024];
    h5tools_str_t buffer;          /* string into which to render   */
    h5tools_context_t ctx;            /* print context  */
    h5tool_format_t  *outputformat = &h5tools_dataformat;
    h5tool_format_t   string_dataformat;
    hsize_t     curr_pos = 0;        /* total data element position   */

    if ((gcpl_id = H5Gget_create_plist(gid)) < 0) {
        SPRTF("error in getting group creation property list ID\n");
        return; // h5tools_setstatus(EXIT_FAILURE);
    }

    /* query the group creation properties for attributes */
    if (H5Pget_attr_creation_order(gcpl_id, &attr_crt_order_flags) < 0) {
        SPRTF("error in getting group creation properties\n");
        return; // h5tools_setstatus(EXIT_FAILURE);
    }

    /* query the group creation properties */
    if(H5Pget_link_creation_order(gcpl_id, &crt_order_flags) < 0) {
        SPRTF("error in getting group creation properties\n");
        return; //h5tools_setstatus(EXIT_FAILURE);
    }

    if(H5Pclose(gcpl_id) < 0) {
        SPRTF("error in closing group creation property list ID\n");
        return; // h5tools_setstatus(EXIT_FAILURE);
    }

    /* setup */
    HDmemset(&buffer, 0, sizeof(h5tools_str_t));

    HDmemset(&ctx, 0, sizeof(ctx));
    //ctx.indent_level = dump_indent/COL;
    //ctx.cur_column = dump_indent;
    
    string_dataformat = *outputformat;

    //if (fp_format) {
    //    string_dataformat.fmt_double = fp_format;
    //    string_dataformat.fmt_float = fp_format;
    //}

    if (h5tools_nCols==0) {
        string_dataformat.line_ncols = 65535;
        string_dataformat.line_per_line = 1;
    }
    else
        string_dataformat.line_ncols = h5tools_nCols;

    //string_dataformat.do_escape = display_escape;
    outputformat = &string_dataformat;

    //ctx.need_prefix = TRUE;

    /* Render the element */
    h5tools_str_reset(&buffer);
    h5tools_str_append(&buffer, "%s \"%s\" %s",
                        h5tools_dump_header_format->groupbegin, name,
                        h5tools_dump_header_format->groupblockbegin);
    h5tools_render_element(rawoutstream, outputformat, &ctx, &buffer, &curr_pos, (size_t)outputformat->line_ncols, (hsize_t)0, (hsize_t)0);
    
    ctx.indent_level++;
    //dump_indent += COL;

    if(!HDstrcmp(name, "/") && unamedtype) {
        unsigned    u;  /* Local index variable */

        /* dump unamed type in root group */
        for(u = 0; u < type_table->nobjs; u++)
            if(!type_table->objs[u].recorded) {
                dset = H5Dopen2(gid, type_table->objs[u].objname, H5P_DEFAULT);
                type = H5Dget_type(dset);
                sprintf(type_name, "#"H5_PRINTF_HADDR_FMT, type_table->objs[u].objno);
                dump_function_table->dump_named_datatype_function(type, type_name);
                H5Tclose(type);
                H5Dclose(dset);
            }
    } /* end if */

    //if(display_oid) {
    //    h5tools_dump_oid(rawoutstream, outputformat, &ctx, gid);
    //}

    //h5tools_dump_comment(rawoutstream, outputformat, &ctx, gid);

    H5Oget_info(gid, &oinfo);

    /* Must check for uniqueness of all objects if we've traversed an elink,
     * otherwise only check if the reference count > 1.
     */
    //if(oinfo.rc > 1 || hit_elink) {
    if(oinfo.rc > 1) {
        obj_t  *found_obj;    /* Found object */

        found_obj = search_obj(group_table, oinfo.addr);

        if (found_obj == NULL) {
            SPRTF("internal error (file %s:line %d)\n", __FILE__, __LINE__);
            return; // h5tools_setstatus(EXIT_FAILURE);
        }
        else if (found_obj->displayed) {
            ctx.need_prefix = TRUE;

            /* Render the element */
            h5tools_str_reset(&buffer);
            h5tools_str_append(&buffer, "%s \"%s\"", HARDLINK, found_obj->objname);
            h5tools_render_element(rawoutstream, outputformat, &ctx, &buffer, &curr_pos, (size_t)outputformat->line_ncols, (hsize_t)0, (hsize_t)0);
        }
        else {
            found_obj->displayed = TRUE;
            //attr_iteration(gid, attr_crt_order_flags);
            //link_iteration(gid, crt_order_flags);
        }
    }
    else {
        //attr_iteration(gid, attr_crt_order_flags);
        //link_iteration(gid, crt_order_flags);
    }

    //dump_indent -= COL;
    ctx.indent_level--;

    ctx.need_prefix = TRUE;
    h5tools_simple_prefix(rawoutstream, outputformat, &ctx, (hsize_t)0, 0);

    /* Render the element */
    h5tools_str_reset(&buffer);
    if(HDstrlen(h5tools_dump_header_format->groupblockend)) {
        h5tools_str_append(&buffer, "%s", h5tools_dump_header_format->groupblockend);
        if(HDstrlen(h5tools_dump_header_format->groupend))
            h5tools_str_append(&buffer, " ");
    }
    if(HDstrlen(h5tools_dump_header_format->groupend))
        h5tools_str_append(&buffer, "%s", h5tools_dump_header_format->groupend);
    h5tools_render_element(rawoutstream, outputformat, &ctx, &buffer, &curr_pos, (size_t)outputformat->line_ncols, (hsize_t)0, (hsize_t)0);

    h5tools_str_close(&buffer);
}

/* Standard DDL output */
//static const dump_functions ddl_function_table = {
//    dump_group,
//    dump_named_datatype,
//    dump_dataset,
//    dump_dataspace,
//    dump_datatype,
//    dump_attr_cb,
//    dump_data
//};

static const dump_functions ddl_function_table = {
    dump_group,
    0,
    0,
    0,
    0,
    0,
    0
};


int open_hdf_file(char *in_file)
{
    int iret = 0;
    hid_t fid, dset;
    hid_t gid = -1;

    H5O_info_t oi;
    unsigned u;

    dump_function_table = &ddl_function_table;

    /*
     * Open file and dataset using the default properties.
     */
    fid = H5Fopen (in_file, H5F_ACC_RDONLY, H5P_DEFAULT);
    if (fid < 0) {
        SPRTF("%s: failed to open '%s'!\n", module, in_file);
        return 1;
    }
    if(H5Oget_info_by_name(fid, "/", &oi, H5P_DEFAULT) < 0) {
        SPRTF("%s: get_info_by_name for root in '%s'!\n", module, in_file);
        return 1;
    }

    group_table = table_list.tables[0].group_table;
    dset_table = table_list.tables[0].dset_table;
    type_table = table_list.tables[0].type_table;

    /* does there exist unamed committed datatype */
    for (u = 0; u < type_table->nobjs; u++) {
        if (!type_table->objs[u].recorded) {
            unamedtype = 1;
            break;
        } /* end if */
    }

    if((gid = H5Gopen2(fid, "/", H5P_DEFAULT)) < 0) {
        SPRTF("%s: failed get root group open '%s'!\n", module, in_file);
        return 1;
    }

    if (dump_function_table && dump_function_table->dump_group_function) {
        dump_function_table->dump_group_function(gid, "/" );
    }


    //file = H5Fopen (in_file, H5F_ACC_RDWR, H5P_DEFAULT);
    //dset = H5Dopen (fid, DATASET, H5P_DEFAULT);
    return iret;
}

int check_hdf_file()
{
    char *in_file = (char *)usr_input;
    if (is_file_or_directory(in_file) != 1) {
        SPRTF("%s: Failed to 'stat' file '%s'!\n", module, in_file);
        return 1;
    }
    size_t len = get_last_file_size();
    SPRTF("%s: Opening file '%s',\n %lu (0x%lX) bytes\n", module, in_file, len, len);
    return open_hdf_file(in_file);

}

int check_hdf_file2()
{
    uint32_t *pu32;
    char *file = (char *)usr_input;
    if (is_file_or_directory(file) != 1) {
        SPRTF("%s: Failed to 'stat' file '%s'!\n", module, file);
        return 1;
    }
    size_t len = get_last_file_size();
    size_t remlen = len;
    SPRTF("%s: Opening file '%s',\n %lu (0x%lX) bytes\n", module, file, len, len);
    FILE *fp = fopen(file,"rb");
    if (!fp) {
        SPRTF("%s: Failed to open file '%s'!\n", module, file);
        return 1;
    }
    // Format Signature (8 bytes)
    size_t read = MX_READ;
    if (remlen < read) {
        read = remlen;
    }
    size_t res = fread(read_buff,read,1,fp);
    if (res != 1) {
        fclose(fp);
        SPRTF("%s: Failed to read %ld bytes from file '%s'!\n", module, read, file);
        return 1;
    }

    PHDRSB0 phdr0 = (PHDRSB0)read_buff;

    if (memcmp(phdr0->sig,hdfsig,8)) {
        fclose(fp);
        SPRTF("%s: Signature of file '%s' does NOT match!\n", module, file);
        return 1;
    }

    // *** DEBUG ONLY ***
    show_hdr_sizes();
    HexDump(read_buff,read);
    //////////////////////

    uint32_t vers = phdr0->Sbver;
    uint64_t end = phdr0->end;
    size_t soof = phdr0->soof;
    size_t sool = phdr0->sool;
    uint8_t *ptr = read_buff;
    uint16_t *p16 = (uint16_t *)ptr;
    uint32_t *p32 = (uint32_t *)ptr;
    uint64_t *p64 = (uint64_t *)ptr;
    uint32_t u32;
    SPRTF("%s: Version Number of the Superblock %u - ", module, vers);
    if (vers == 2) {
        PHDRSB2 ph2 = (PHDRSB2)read_buff;
        end = ph2->end;
        soof = ph2->soof;
        sool = ph2->sool;
        SPRTF("soof %lu, sool %lu - ok\n", soof, sool);
        SPRTF("%s: End of File Address ", module );
        SPRTF(" %" UI64, end);
        SPRTF(" 0x%" UX64, end);
        if ((uint64_t)len == end) {
            SPRTF(" - ok\n");
        } else {
            SPRTF(" - End does NOT match size!\n");
            fclose(fp);
            return 1;
        }
        ph2++;
        ptr = (uint8_t *) ph2;
        POBJHDR poh = (POBJHDR)ptr;
        if ((ptr[0] == 'O') && (ptr[1] == 'H') && (ptr[2] == 'D') &&
            (ptr[3] == 'R') && (ptr[4] == 2)) {
            uint8_t flag = poh->flags;
            SPRTF("%s: Flag ", module);
            show_flag_8( flag );
            SPRTF("\n");
            uint8_t sz = (flag & 3);        // bits 0-1
            uint8_t b4 = (flag & 0x10);
            uint8_t tms = (flag & 0x20);    // bit 5
            poh++;
            ptr = (uint8_t *)poh;
            if (tms) {
                poh++;
                POBJTIMES pot = (POBJTIMES)ptr;
                SPRTF("Times: acc ");
                if (show_gmtime( pot->acces )) {
                    SPRTF(" mod ");
                    if (show_gmtime( pot->modtm )) {

                    }
                }
                SPRTF("\n");
                pot++;
                ptr = (uint8_t *)pot;
            }
            if (b4) {
                pu32 = (uint32_t *)ptr;
                pu32++;
                pu32++;
                ptr = (uint8_t *)pu32;
            }
            switch (sz) {
            case 0:
                u32 = *ptr;
                SPRTF("Size of chunk is 1 byte %u\n", u32);
                ptr++;
                break;
            case 1:
                p16 = (uint16_t *)ptr;
                u32 = *p16;
                SPRTF("Size of chunk is 2 bytes %u\n", u32);
                p16++;
                ptr = (uint8_t *)p16;
                break;
            case 2:
                p32 = (uint32_t *)ptr;
                u32 = *p32;
                SPRTF("Size of chunk is 4 bytes %u\n", u32);
                p32++;
                ptr = (uint8_t *)p32;
                break;
            case 3:
                p64 = (uint64_t *)ptr;
                u32 = (uint32_t)*p64;
                SPRTF("Size of chunk is 8 bytes %u\n", u32);
                p64++;
                ptr = (uint8_t *)p64;
                break;
            default:
                SPRTF("Uncased size %ld\n", sz );
                fclose(fp);
                return 1;
            }

            HexDump(ptr,64);

            ptr = 0;
            // ==================

        }
    } else if ((vers == 0)||(vers == 1)) {
        SPRTF("ok\n");
        SPRTF("%s: End of File Address %" UI64 "\n", module, end );
    } else {
        SPRTF("Unsupported version!\n");
        fclose(fp);
        return 1;
    }
    fclose(fp);
    return 0;
}

// for DEBUG
// PATH=F:\Projects\software\bin;%PATH%
// main() OS entry
int main( int argc, char **argv )
{
    int iret = 0;
    set_log_file((char *)def_log, false);
    iret = parse_args(argc,argv);
    if (iret)
        return iret;

    iret = check_hdf_file(); // TODO: actions of app

    return iret;
}


// eof = hdfData.cxx
