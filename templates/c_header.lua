local name = JSON_NAME
local ucname = string.upper(name)
local include_guard = ucname .. "_H_INCLUDED_"

local function append_line(t, indent, text)
    t[#t+1] = string.rep('\t', indent) .. text .. '\n'
end

local function newline(t)
    t[#t+1] = '\n'
end

local function append(t, text)
    t[#t+1] = text
end

local function gen_struct(obj, indent)
    local res = { }

    while obj do
        if(obj.type == 'object') then
            append_line(res, indent, "struct {")
            append(res, gen_struct(obj.children, indent+1))
            append_line(res, indent, "} " .. obj.name .. ";")
        elseif(obj.type == 'any') then
            append_line(res, indent, "struct json_obj_any " .. obj.name .. ";")
        else
            append_line(res, indent, obj.ctype .. " " .. obj.name .. ";")
        end
        append_line(res, indent, "int is_set_" .. obj.name .. ";")

        obj = obj.next
    end
    
    return table.concat(res)
end

local output = {
"#ifndef ", include_guard, "\n",
"#define ", include_guard, "\n",
"\n",
"#include <jsonparsergen.h>\n",
"\n",
"struct ", name, " {\n",
    gen_struct(JSON_ROOT, 1),
"};\n",
"\n",
"char* ", name, "_pack(struct ", name, "*);\n",
"int ", name, "_unpack(struct ", name, "*, const char* data);\n",
"void ", name, "_cleanup(struct ", name, "*);\n",
"\n",
"#endif /* ", include_guard, " */\n",
}

print(table.concat(output))

