#include <stdio.h>
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

void codegen_header(const char* name, const struct obj* obj)
{
    printf("struct %s {\n", name);
    gen_list(obj, 4);
    printf("};\n\n");

    gen_prototypes(name);
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

char* codegen_pack(const char* name, const struct obj* obj)
{
    return NULL;
}

void codegen_source(const char* name, const struct obj* obj)
{
    codegen_cleanup(name, obj);
}

