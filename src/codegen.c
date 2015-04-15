#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "obj.h"

void gen_struct(const char* name, const struct obj* obj, int indent);

void gen_list(const struct obj* obj, int indent)
{
    if(obj->type == OBJ_OBJECT)
    {
        gen_struct(obj->name, obj, indent);
    }
    else if(obj->type == OBJ_ANY)
    {
        printf("%*sstruct json_obj_any %s;\n", indent, "", obj->name);
    }
    else
    {
        if(obj->length == 1)
            printf("%*s%s %s;\n", indent, "", obj_strctype(obj), obj->name);
        else
            printf("%*s%s %s[%d];\n", indent, "", obj_strctype(obj), obj->name,
                                      obj->length);
    }
    printf("%*sint is_set_%s;\n", indent, "", obj->name);

    if(obj->next)
        gen_list(obj->next, indent);
}

void gen_struct(const char* name, const struct obj* obj, int indent)
{
    printf("%*sstruct {\n", indent, "");
    gen_list(obj->children, indent+4);
    printf("%*s} %s;\n", indent, "", obj->name);
}

void gen_prototypes(const char* name)
{
    printf("char* %s_pack(struct %s*);\n", name, name);
    printf("int %s_unpack(struct %s*, const char* data);\n", name, name);
    printf("void %s_cleanup(struct %s*);\n", name, name);
    printf("\n");
}

static inline void strtoupper(char* dst, const char* src, size_t dst_size)
{
    int i;
    for(i = 0; i < dst_size-1 && src[i]; ++i)
        dst[i] = toupper(src[i]);
    dst[i] = 0;
}

void codegen_header(const char* name, const struct obj* obj)
{
    char ucname[256];
    strtoupper(ucname, name, sizeof(ucname));

    printf("#ifndef %s_H_INCLUDED_\n", ucname);
    printf("#define %s_H_INCLUDED_\n\n", ucname);

    printf("#include <jsonparsergen.h>\n\n");

    printf("struct %s {\n", name);
    gen_list(obj, 4);
    printf("};\n\n");

    gen_prototypes(name);

    printf("#endif /* %s_H_INCLUDED_ */\n", ucname);
}

void gen_cleanup(const struct obj* obj, int indent, const char* prefix)
{
    char prefix_buffer[256];

    if(obj->type == OBJ_OBJECT)
    {
        printf("%*s    if(obj->%sis_set_%s)\n",
               indent, "", prefix ? prefix : "", obj->name);
        printf("%*s    {\n", indent, "");
        snprintf(prefix_buffer, sizeof(prefix_buffer)-1, "%s%s.",
                 prefix ? prefix : "", obj->name);
        gen_cleanup(obj->children, indent+4, prefix_buffer);
        printf("%*s    }\n", indent, "");
    }
    else if(obj->type == OBJ_STRING)
    {
        printf("%*s    if(obj->%sis_set_%s)\n",
               indent, "", prefix ? prefix : "", obj->name);
        printf("%*s        free(obj->%s%s);\n",
               indent, "", prefix ? prefix : "", obj->name);
    }
    else if(obj->type == OBJ_ANY)
    {
        printf("%*s    if(obj->%sis_set_%s)\n",
               indent, "", prefix ? prefix : "", obj->name);
        printf("%*s        if(obj->%s%s.type == JSON_OBJ_STRING)\n",
               indent, "", prefix ? prefix : "", obj->name);
        printf("%*s            free(obj->%s%s.string_);\n",
               indent, "", prefix ? prefix : "", obj->name);
    }

    if(obj->next)
        gen_cleanup(obj->next, indent, prefix);
}

void codegen_cleanup(const char* name, const struct obj* obj)
{
    printf("void %s_cleanup(struct %s* obj)\n{\n", name, name);
    gen_cleanup(obj, 0, NULL);
    printf("}\n\n");
}

void gen_pack(const struct obj* obj, const char* prefix)
{
    char prefix_buffer[256];

    if(obj->is_optional)
        printf("    if(obj->is_set_%s%s) {\n", prefix ? prefix : "", obj->name);

    if(obj->type != OBJ_ANY)
    {
        /* TODO: Make buffer dynamic */
        printf("    if(i >= buffer_size) goto failure;\n");
        printf("    i += snprintf(&buffer[i], buffer_size-i, \"\\\"%s\\\"\");\n",
               obj->name);
    }

    switch(obj->type)
    {
    case OBJ_UNINITIALIZED:
        abort();
        break;
    case OBJ_STRING:
        printf("    i += snprintf(&buffer[i], buffer_size-i, \":\\\"%%s\\\"\", obj->%s%s);\n",
               prefix ? prefix : "", obj->name);
        break;
    case OBJ_INTEGER:
        printf("    i += snprintf(&buffer[i], buffer_size-i, \":%%lld\", obj->%s%s);\n",
               prefix ? prefix : "", obj->name);
        break;
    case OBJ_REAL:
        printf("    i += snprintf(&buffer[i], buffer_size-i, \":%%e\", obj->%s%s);\n",
               prefix ? prefix : "", obj->name);
        break;
    case OBJ_BOOL:
        printf("    i += snprintf(&buffer[i], buffer_size-i, \":%%s\", obj->%s%s ? \"true\" : \"false\");\n",
               prefix ? prefix : "", obj->name);
        break;
    case OBJ_OBJECT:
        printf("    i += snprintf(&buffer[i], buffer_size-i, \":{\");\n");
        snprintf(prefix_buffer, sizeof(prefix_buffer)-1, "%s%s.",
                 prefix ? prefix : "", obj->name);
        gen_pack(obj->children, prefix_buffer);
        printf("    i += snprintf(&buffer[i], buffer_size-i, \"}\");\n");
        break;
    case OBJ_ANY:
        break;
    }

    struct obj* tail = obj->next;
    if(tail)
    {
        if(obj->type != OBJ_ANY)
        {
            printf("    buffer[i++] = ',';\n");
            if(obj->is_optional)
                printf("    }\n");
        }
        gen_pack(tail, prefix);
    }
    else if(obj->type != OBJ_ANY && obj->is_optional)
        printf("    }\n");
}

void gen_validate(const struct obj* obj, int indent, const char* prefix)
{
    char prefix_buffer[256];

    if(!obj->is_optional)
        printf("%*s    if(!obj->%sis_set_%s) return -1;\n",
               indent, "", prefix ? prefix : "", obj->name);

    if(obj->type == OBJ_OBJECT)
    {
        printf("%*s    if(obj->%sis_set_%s))\n",
               indent, "", prefix ? prefix : "", obj->name);
        printf("%*s    {\n", indent, "");
        snprintf(prefix_buffer, sizeof(prefix_buffer)-1, "%s%s.",
                 prefix ? prefix : "", obj->name);
        gen_validate(obj->children, indent+4, prefix_buffer);
        printf("%*s    }\n", indent, "");
    }

    struct obj* tail = obj->next;
    if(tail)
        gen_validate(tail, indent, prefix);
}

void codegen_validate(const char* name, const struct obj* obj)
{
    printf("static int validate(const struct %s* obj)\n{\n", name);
    gen_validate(obj, 0, NULL);
    printf("    return 0;\n");
    printf("}\n\n");
}

void codegen_pack(const char* name, const struct obj* obj)
{
    printf("char* %s_pack(struct %s* obj)\n{\n", name, name);
    printf("\
    size_t buffer_size = 4096;\n\
    char* buffer = malloc(buffer_size);\n\
    if(!buffer)\n\
        return NULL;\n\
    int i = 0;\n\
");
    printf("    i += snprintf(&buffer[i], buffer_size-i, \"{\");\n");
    gen_pack(obj, NULL);
    printf("    i += snprintf(&buffer[i], buffer_size-i, \"}\");\n");
    printf("\
    return buffer;\n\
\n\
failure:\n\
    free(buffer);\n\
    return NULL;\n\
}\n\n\
");
}

void gen_newstring()
{
    printf("\
static inline char* new_string(const char* src, size_t len)\n\
{\n\
    char* dst = malloc(len+1);\n\
    if(!dst)\n\
        return NULL;\n\
    memcpy(dst, src, len);\n\
    dst[len] = 0;\n\
    return dst;\n\
}\n\n\
");
}

void gen_decode_any()
{
    printf("\
static int decode_any(struct json_obj_any* dst, struct json_obj* json,\n\
                      const char* value, size_t value_length)\n\
{\n\
    dst->type = json->type;\n\
    switch(json->type)\n\
    {\n\
    case JSON_OBJ_NULL:\n\
        break;\n\
    case JSON_OBJ_OBJ:\n\
    case JSON_OBJ_ARRAY:\n\
        dst->obj_start = json->value.start;\n\
        dst->obj_end = json->value.end;\n\
        break;\n\
    case JSON_OBJ_NUMBER:\n\
        dst->integer = strtoll(value, NULL, 0);\n\
        dst->real = strtod(value, NULL);\n\
        break;\n\
    case JSON_OBJ_STRING:\n\
        dst->string_ = new_string(value, value_length);\n\
        break;\n\
    case JSON_OBJ_TRUE:\n\
        dst->type = JSON_OBJ_BOOL;\n\
        dst->bool_ = 1;\n\
        break;\n\
    case JSON_OBJ_FALSE:\n\
        dst->type = JSON_OBJ_BOOL;\n\
        dst->bool_ = 0;\n\
        break;\n\
    default:\n\
        break;\n\
    }\n\
    return 0;\n\
}\n\n\
");
}

void gen_unpack(const struct obj* obj, int indent, int level,
                const char* prefix)
{
    char prefix_buffer[256];

    printf("%*sif(key_length == %u && 0 == strncmp(key, \"%s\", %u))\n",
           indent, "", strlen(obj->name), obj->name, strlen(obj->name));
    printf("%*s{\n", indent, "");
    printf("%*s    if(obj%s%sis_set_%s) goto failure;\n",
           indent, "", prefix, level == 0 ? "->" : ".", obj->name);
    switch(obj->type)
    {
    case OBJ_ANY:
        printf("%*s    if(decode_any(&obj%s%s%s, json, value, value_length) < 0) goto failure;\n",
               indent, "", prefix, level == 0 ? "->" : ".", obj->name);
        break;
    case OBJ_STRING:
        printf("%*s    if(json->type != JSON_OBJ_STRING) goto failure;\n",
               indent, "");
        printf("%*s    obj%s%s%s = new_string(value, value_length);\n",
               indent, "", prefix, level == 0 ? "->" : ".", obj->name);
        break;
    case OBJ_INTEGER:
        printf("%*s    if(json->type != JSON_OBJ_NUMBER) goto failure;\n",
               indent, "");
        printf("%*s    obj%s%s%s = strtoll(value, NULL, 0);\n",
               indent, "", prefix,level == 0 ? "->" : ".", obj->name);
        break;
    case OBJ_REAL:
        printf("%*s    if(json->type != JSON_OBJ_NUMBER) goto failure;\n",
               indent, "");
        printf("%*s    obj%s%s%s = strtod(value, NULL);\n",
               indent, "", prefix, level == 0 ? "->" : ".", obj->name);
        break;
    case OBJ_BOOL:
        printf("%*s    if(json->type != JSON_OBJ_TRUE && json->type != JSON_OBJ_FALSE) goto failure;\n",
               indent, "");
        printf("%*s    obj%s%s%s = value->type != JSON_OBJ_FALSE;\n",
               indent, "", prefix, level == 0 ? "->" : ".", obj->name);
        break;
    case OBJ_OBJECT:
        printf("%*s    if(json->type != JSON_OBJ_OBJ) goto failure;\n",
               indent, "");
        printf("%*s    struct json_obj* tmp_json%d = json;\n", indent, "", level);
        printf("%*s    for(json = json->children; json; json = json->next)\n",
               indent, "");
        printf("%*s    {\n", indent, "");
        printf("%*s        key = &data[json->key.start];\n", indent, "");
        printf("%*s        key_length = json->key.end - json->key.start;\n",
               indent, "");
        printf("%*s        value = &data[json->value.start];\n", indent, "");
        printf("%*s        value_length = json->value.end - json->value.start;\n\n",
               indent, "");
        snprintf(prefix_buffer, sizeof(prefix_buffer)-1, "%s%s%s", prefix,
                 level == 0 ? "->" : ".", obj->name);
        gen_unpack(obj->children, indent+8, level+1, prefix_buffer);
        printf("%*s    }\n", indent, "");
        printf("%*s    json = tmp_json%d;\n", indent, "", level);
        break;
    case OBJ_UNINITIALIZED:
        abort();
        break;
    }
    printf("%*s    obj%s%sis_set_%s = 1;\n",
           indent, "", prefix, level == 0 ? "->" : ".", obj->name);
    printf("%*s}\n", indent, "");

    const struct obj* tail = obj->next;
    if(tail)
    {
        printf("%*selse\n", indent, "");
        gen_unpack(tail, indent, level, prefix);
    }
}

void codegen_unpack(const char* name, const struct obj* obj)
{
    printf("int %s_unpack(struct %s* obj, const char* data)\n{\n", name, name);
    printf("    memset(obj, 0, sizeof(struct %s));\n", name);
    printf("\
    const char *key, *value;\n\
    size_t key_length, value_length;\n\
    struct json_obj *json, *json_root = json_lexer(data);\n\
    if(!json_root)\n\
        return -1;\n\
    for(json = json_root; json; json = json->next)\n\
    {\n\
        key = &data[json->key.start];\n\
        key_length = json->key.end - json->key.start;\n\
        value = &data[json->value.start];\n\
        value_length = json->value.end - json->value.start;\n\n\
");
    gen_unpack(obj, 8, 0, "");
    printf("\
    }\n\
    if(validate(obj) < 0)\n\
        goto failure;\n\
\n\
    json_obj_free(json_root);\n\
    return 0;\n\
\n\
failure:\n\
    json_obj_free(json_root);\n\
");
    printf("    %s_cleanup(obj);\n", name);
    printf("\
    return -1;\n\
}\n\n");
}

void codegen_util()
{
    gen_newstring();
    gen_decode_any();
}

void codegen_source_head(const char* name)
{
    printf("\
#include <stdio.h>\n\
#include <stdlib.h>\n\
#include <string.h>\n\
");
    printf("#include \"%s.h\"\n\n", name);
}

void codegen_json_info()
{
    printf("\
struct json_obj* json_lexer(const char*);\n\
\n\
");
}

void codegen_source(const char* name, const struct obj* obj)
{
    codegen_source_head(name);
    codegen_json_info();
    codegen_util();
    codegen_validate(name, obj);
    codegen_cleanup(name, obj);
    codegen_pack(name, obj);
    codegen_unpack(name, obj);
}

