#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "obj.h"

void gen_struct(const char* name, const struct obj* obj, int indent);

void gen_list(const struct obj* obj, int indent)
{
    if(obj->type == OBJECT)
    {
        gen_struct(obj->name, obj, indent);
    }
    else
    {
        if(obj->length == 1)
            printf("%*s%s %s;\n", indent, "", obj_strctype(obj), obj->name);
        else
            printf("%*s%s %s[%d];\n", indent, "", obj_strctype(obj), obj->name,
                                      obj->length);
    }

    if(obj->next)
        gen_list(obj->next, indent);
}

void gen_struct(const char* name, const struct obj* obj, int indent)
{
    printf("%*sstruct {\n", indent, "");
    gen_list(obj_children((struct obj*)obj), indent+4);
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

    printf("struct %s {\n", name);
    gen_list(obj, 4);
    printf("};\n\n");

    gen_prototypes(name);

    printf("#endif /* %s_H_INCLUDED_ */\n", ucname);
}

void gen_cleanup(const struct obj* obj, int level)
{
    if(obj->type == OBJECT)
    {
        printf("%*s    {\n", level*4, "");
        printf("%*s        void* tmp%d = obj;\n", level*4, "", level);
        printf("%*s        obj = obj->%s;\n", level*4, "", obj->name);
        gen_cleanup(obj_children((struct obj*)obj), level+1);
        printf("%*s        obj = tmp%d;\n", level*4, "", level);
        printf("%*s    }\n", level*4, "");
    }

    if(obj->type == STRING)
        printf("%*s    free(obj->%s);\n", level*4, "", obj->name);

    if(obj->next)
        gen_cleanup(obj->next, level);
}

void codegen_cleanup(const char* name, const struct obj* obj)
{
    printf("void %s_cleanup(struct %s* obj)\n{\n", name, name);
    gen_cleanup(obj, 0);
    printf("}\n\n");
}

void gen_pack(const struct obj* obj, int indent, const char* prefix)
{
    char prefix_buffer[256];

    printf("%*s    i += sprintf(&buffer[i], \"\\\"%s\\\"\");\n",
           indent, "", obj->name);
    switch(obj->type)
    {
    case STRING:
        printf("%*s    i += sprintf(&buffer[i], \":\\\"%%s\\\"\", obj->%s%s);\n",
               indent, "", prefix ? prefix : "", obj->name);
        break;
    case INTEGER:
        printf("%*s    i += sprintf(&buffer[i], \":\\\"%%lld\\\"\", obj->%s%s);\n",
               indent, "", prefix ? prefix : "", obj->name);
        break;
    case REAL:
        printf("%*s    i += sprintf(&buffer[i], \":\\\"%%e\\\"\", obj->%s%s);\n",
               indent, "", prefix ? prefix : "", obj->name);
        break;
    case BOOL:
        printf("%*s    i += sprintf(&buffer[i], \":\\\"%%s\\\"\", obj->%s%s ? \"true\" : \"false\");\n",
               indent, "", prefix ? prefix : "", obj->name);
        break;
    case OBJECT:
        printf("%*s    i += sprintf(&buffer[i], \":{\");\n", indent, "");
        snprintf(prefix_buffer, sizeof(prefix_buffer)-1, "%s%s.",
                 prefix ? prefix : "", obj->name);
        gen_pack(obj_children((struct obj*)obj), indent+4, prefix_buffer);
        printf("%*s    buffer[i++] = '}';\n", indent, "");
        break;
    }

    struct obj* tail = obj->next;
    if(tail)
    {
        printf("%*s    buffer[i++] = ',';\n", indent, "");
        gen_pack(tail, indent, prefix);
    }
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
    gen_pack(obj, 0, NULL);
    printf("    return buffer;\n");
    printf("}\n\n");
}

void gen_newstring()
{
    printf("\
static inline char* new_string(const char* src, size_t len)\n\
{\n\
    char* dst = malloc(len+1);\n\
    if(!dst)\n\
        return NULL;\n\
    strcpy(dst, str);\n\
    return dst;\n\
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
    switch(obj->type)
    {
    case STRING:
        printf("%*s    obj%s%s%s = new_string(value, value_length);\n",
               indent, "", prefix, level == 0 ? "->" : ".", obj->name);
        break;
    case INTEGER:
        printf("%*s    obj%s%s%s = strtoll(value, NULL, 0);\n",
               indent, "", prefix,level == 0 ? "->" : ".", obj->name);
        break;
    case REAL:
        printf("%*s    obj%s%s%s = strtod(value, NULL);\n",
               indent, "", prefix, level == 0 ? "->" : ".", obj->name);
        break;
    case BOOL:
        printf("%*s    obj%s%s%s = value->type != JSON_OBJ_FALSE && value->type != JSON_OBJ_NULL;\n",
               indent, "", prefix, level == 0 ? "->" : ".", obj->name);
        break;
    case OBJECT:
        printf("%*s    if(json->type != JSON_OBJ_OBJ)\n", indent, "");
        printf("%*s        continue;\n", indent, "");
        printf("%*s    struct json_obj* tmp_json%d = json;\n", indent, "", level);
        printf("%*s    for(json = json->children; json; json = json->next)\n",
               indent, "");
        printf("%*s    {\n", indent, "");
        printf("%*s        key = &data[json->key.start];\n", indent, "");
        printf("%*s        key_length = json->key.stop - json->key.start;\n",
               indent, "");
        printf("%*s        value = &data[json->value.start];\n", indent, "");
        printf("%*s        value_length = json->value.stop - json->value.start;\n\n",
               indent, "");
        snprintf(prefix_buffer, sizeof(prefix_buffer)-1, "%s%s%s", prefix,
                 level == 0 ? "->" : ".", obj->name);
        gen_unpack(obj_children((struct obj*)obj), indent+8, level+1, prefix_buffer);
        printf("%*s    }\n", indent, "");
        printf("%*s    json = tmp_json%d;\n", indent, "", level);
        break;
    }
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
    printf("\
    char *key, *value;\n\
    size_t key_length, value_length;\n\
    struct json_obj *json, *json_root = json_lexer(data);\n\
    if(!json_root)\n\
        return -1;\n\
    for(json = json_root; json; json = json->next)\n\
    {\n\
        key = &data[json->key.start];\n\
        key_length = json->key.stop - json->key.start;\n\
        value = &data[json->value.start];\n\
        value_length = json->value.stop - json->value.start;\n\n\
");
    gen_unpack(obj, 8, 0, "");
    printf("    }\n");
    printf("    json_obj_free(json_root);\n");
    printf("}\n\n");
}

void codegen_util()
{
    gen_newstring();
}

void codegen_source(const char* name, const struct obj* obj)
{
    codegen_util();
    codegen_cleanup(name, obj);
    codegen_pack(name, obj);
    codegen_unpack(name, obj);
}

