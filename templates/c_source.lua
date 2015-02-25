local name = JSON_NAME

local function indent(level)
    return string.rep('    ', level)
end

local function append_line(t, level, text)
    t[#t+1] = indent(level) .. text .. '\n'
end

local function newline(t)
    t[#t+1] = '\n'
end

local function append(t, text)
    t[#t+1] = text
end

local function match(x)
    return function(t) return t[x] or t._ end
end

local function get_current_value(prefix, name)
    if prefix then
        return "obj->" .. prefix .. '.' .. name
    else 
        return "obj->" .. name
    end
end

local function get_current_isset(prefix, name)
    if prefix then
        return "obj->" .. prefix .. '.is_set_' .. name
    else 
        return "obj->is_set_" .. name
    end
end

local function get_new_prefix(prefix, name)
    if prefix then
        return prefix .. '.' .. name
    else
        return name
    end
end



local function gen_validate(obj, level, prefix)
    local res = { }

    while obj do
        if not obj.is_optional then
            append_line(res, level, "if(!" .. get_current_isset(prefix, obj.name) .. ") goto failure;")
        end

        if(obj.type == 'object') then
            if(obj.is_optional) then
                append_line(res, level, "if(" .. get_current_isset(prefix, obj.name) .. ")")
                append_line(res, level, "{")
                append(res, gen_validate(obj.children, level+1, get_new_prefix(prefix, obj.name)))
                append_line(res, level, "}")
            else
                append(res, gen_validate(obj.children, level, get_new_prefix(prefix, obj.name)))
            end
        end

        obj = obj.next
    end

    return table.concat(res)
end

local function gen_unpack(obj, level, prefix)
    local res = { }

    while obj do
        append_line(res, level, "if(key_length == " .. string.len(obj.name)
                                                    .. ' && 0 == strncmp(key, "'
                                                    .. obj.name .. '", '
                                                    .. string.len(obj.name) .. "))")
        append_line(res, level, "{")

        level = level + 1
        append(res, table.concat(match(obj.type) {
            object = function() return {
                indent(level), "if(json->type != JSON_OBJ_OBJ) goto failure;\n",
                indent(level), "struct json_obj* tmp_json", level, " = json;\n",
                indent(level), "for(json = json->children; json; json = json->next)\n",
                indent(level), "{\n",
                indent(level), "    key = &data[json->key.start];\n",
                indent(level), "    key_length = json->key.end - json->key.start;\n",
                indent(level), "    value = &data[json->value.start];\n",
                indent(level), "    value_length = json->value.end - json->value.start;\n\n",
                gen_unpack(obj.children, level+1, get_new_prefix(prefix, obj.name)),
                indent(level), "}\n",
                indent(level), "json = tmp_json", level, ";\n"
            } end,
            any = function() return {
                indent(level), "if(decode_any(&", get_current_value(prefix, obj.name),
                                    ", json, value, value_length) < 0) goto failure;\n"
            } end,
            ['string'] = function() return {
                indent(level), "if(json->type != JSON_OBJ_STRING) goto failure;\n",
                indent(level), get_current_value(prefix, obj.name), " = new_string(value, value_length);\n"
            } end,
            int = function() return {
                indent(level), "if(json->type != JSON_OBJ_NUMBER) goto failure;\n",
                indent(level), get_current_value(prefix, obj.name), " = strtoll(value, NULL, 0);\n"
            } end,
            real = function() return {
                indent(level), "if(json->type != JSON_OBJ_NUMBER) goto failure;\n",
                indent(level), get_current_value(prefix, obj.name), " = strtod(value, NULL);\n"
            } end,
            bool = function() return {
                indent(level), "if(json->type != JSON_OBJ_TRUE && json->type != JSON_OBJ_FALSE) goto failure;\n",
                indent(level), get_current_value(prefix, obj.name), " = value->type != JSON_OBJ_FALSE;\n"
            } end,
            _ = function() return { 'ERROR!' } end
        } ()))

        append_line(res, level, get_current_isset(prefix, obj.name) .. " = 1;")
        level = level - 1

        append_line(res, level, "}")

        obj = obj.next
    end

    
    return table.concat(res)
end

local function gen_cleanup(obj, level, prefix)
    local res = { }

    while obj do
        match(obj.type) {
            object = function()
                append_line(res, level, "if(" .. get_current_isset(prefix, obj.name) .. ")")
                append_line(res, level, "{")
                append(res, gen_cleanup(obj.children, level+1, get_new_prefix(prefix, name)))
                append_line(res, level, "}")
            end,
            string = function()
                append_line(res, level, "if(" .. get_current_isset(prefix, obj.name) .. ")")
                append_line(res, level, "    free(" .. get_current_value(prefix, obj.name) .. ");")
            end,
            any = function()
                append_line(res, level, "if(" .. get_current_isset(prefix, obj.name) .. " && " 
                                              .. get_current_value(prefix, obj.name) ..".type == JSON_OBJ_STRING)")
                append_line(res, level, "    free(" .. get_current_value(prefix, obj.name) .. ");")
            end,
            _ = function() end
        } ()

        obj = obj.next
    end

    return table.concat(res)
end

local output = {
[[#include <stdio.h>
#include <stdlib.h>
#include <string.h>

]],
'#include "', name, '.h"', [[


static inline char* new_string(const char* src, size_t len)
{
    char* dst = malloc(len+1);
    if(!dst)
        return NULL;
    memcpy(dst, src, len);
    dst[len] = 0;
    return dst;
}

static int decode_any(struct json_obj_any* dst, struct json_obj* json,
                      const char* value, size_t value_length)
{
    dst->type = json->type;
    switch(json->type)
    {
    case JSON_OBJ_NULL:
        break;
    case JSON_OBJ_OBJ:
    case JSON_OBJ_ARRAY:
        dst->obj_start = json->value.start;
        dst->obj_end = json->value.end;
        break;
    case JSON_OBJ_NUMBER:
        dst->integer = strtoll(value, NULL, 0);
        dst->real = strtod(value, NULL);
        break;
    case JSON_OBJ_STRING:
        dst->string_ = new_string(value, value_length);
        break;
    case JSON_OBJ_TRUE:
        dst->type = JSON_OBJ_BOOL;
        dst->bool_ = 1;
        break;
    case JSON_OBJ_FALSE:
        dst->type = JSON_OBJ_BOOL;
        dst->bool_ = 0;
        break;
    default:
        break;
    }
    return 0;
}

]],
"void ", name, "_cleanup(struct ", name, "* obj)\n",
"{\n",
    gen_cleanup(JSON_ROOT, 1),
"}\n",
"\n",
"int ", name, "_unpack(struct ", name, [[* obj, const char* data)
{
    memset(obj, 0, sizeof(*obj));
    const char *key, *value;
    size_t key_length, value_length;
    struct json_obj *json, *json_root = json_lexer(data);
    if(!json_root)
        return -1;
    for(json = json_root; json; json = json->next)
    {
        key = &data[json->key.start];
        key_length = json->key.end - json->key.start;
        value = &data[json->value.start];
        value_length = json->value.end - json->value.start;

]], gen_unpack(JSON_ROOT, 2), [[
    }

]], gen_validate(JSON_ROOT, 1), [[

    json_obj_free(json_root);
    return 0;

failure:
    json_obj_free(json_root);
    ]], name, [[_cleanup(obj);
    return -1;\n",
}

]]
}

print(table.concat(output))

