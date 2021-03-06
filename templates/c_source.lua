local function rtrim(s)
    local n = #s
    while n > 0 and s:find("^ ", n) do n = n - 1 end
    return s:sub(1, n)
end

local function indent(text)
    return rtrim('    ' .. string.gsub(text, '\n', '\n    '))
end

local function match(x)
    return function(t) return t[x] or t._ end
end

function copy_table(t)
    local t2 = {}
    for k,v in pairs(t) do
        t2[k] = v
    end
    return t2
end

local function flatten(arr)
    local result = { }

    local function flatten(arr)
        for _, v in ipairs(arr) do
            if type(v) == "table" then
                flatten(v)
            else
                table.insert(result, v)
            end
        end
    end

    flatten(arr)
    return result
end

local function myconcat(sep, ...)
    return table.concat(flatten{...}, sep)
end

local function get_new_prefix(prefix, name)
    if prefix then
        return prefix .. '.' .. name
    else
        return name
    end
end

local function If(cond)
    return 'if(' .. cond .. ')\n'
end

local function ElseIf(cond)
    return 'else if(' .. cond .. ')\n'
end

local function Else(cond)
    return 'else(' .. cond .. ')\n'
end

local function IfThenElse(cond, IF, ELSE)
    return '(' .. cond .. ') ? ' .. IF .. ' : ' .. ELSE
end

local function Switch(var)
    return 'switch(' .. var .. ')\n'
end

local function Case(case, code)
    return 'case ' .. case .. ':\n' .. indent(code .. 'break;\n')
end

local function CodeBlock(code)
    return "{\n" .. indent(table.concat(code)) .. "}\n"
end

local function Not(code)
    return '!' .. code
end

local function Eq(left, right)
    return '(' .. left .. ') == (' .. right .. ')'
end

local function Lt(left, right)
    return left .. ' < ' .. right
end

local function strncmp(left, right, length)
    return 'strncmp(' .. left .. ', ' .. right .. ', ' .. length .. ')'
end

local function Neq(left, right)
    return '(' .. left .. ') != (' .. right .. ')'
end

local function Goto(mark)
    return 'goto ' .. mark .. ';\n'
end

local function Mark(name)
    return name .. ':\n'
end

local function And(...)
    return '(' .. table.concat({...}, ') && (') .. ')'
end

local function Or(...)
    return '(' .. table.concat({...}, ') || (') .. ')'
end

local function Lt(left, right)
    return '(' .. right .. ') < (' .. left ..')'
end

local function For(init, cond, nxt)
    return 'for(' .. init .. '; ' .. cond .. '; ' .. nxt ..')'
end

local function Str(str)
    return '"' .. str .. '"'
end

local function Free(str)
    return "free(" .. str .. ");\n"
end

local function Assign(left, right)
    return left .. ' = ' .. right .. ';\n'
end

local function Isset(prefix, name)
    if prefix then
        return "obj->" .. prefix .. '.is_set_' .. name
    else
        return "obj->is_set_" .. name
    end
end

local function get_current_value(prefix, name)
    if prefix then
        return "obj->" .. prefix .. '.' .. name
    else
        return "obj->" .. name
    end
end

local function get_current_length(prefix, name)
    if prefix then
        return "obj->" .. prefix .. '.lengt_of_' .. name
    else
        return "obj->" .. name
    end
end

local function Append(fmt, ...)
    local t = {...}
    if #t == 0 then
        return 'i += snprintf(&buffer[i], size-i, ' .. Str(fmt) .. ');\n' ..
               If(Eq('i', 'size')) ..
                   indent(Goto('failure'))
    else
        return 'i += snprintf(&buffer[i], size-i, ' .. Str(fmt) .. ', '.. table.concat({...}, ', ') .. ');\n' ..
               If(Eq('i', 'size')) ..
                   indent(Goto('failure'))
   end
end

local function Declare(type, name)
    return type .. ' ' .. name .. ';\n'
end

local function AppendString(value)
    return
        Assign('str', 'json_string_encode(' .. value .. ', strlen(' .. value .. '))') ..
        If(Not('str')) ..
            indent(Goto('failure')) ..
        Append('\\"%s\\"', 'str') ..
        Free('str') ..
        Assign('str', 'NULL')
end

local function gen_pack(obj, prefix)
    local res = { }
    local maybe_comma = IfThenElse(Neq(0, 'comma++'), Str(','), Str(''))

    res[#res+1] = 'comma = 0;\n'

    while obj do
        local key = '\\"' .. obj.name .. '\\"'

        local fn = match(obj.type) {
            ['string'] = function(index) return
                AppendString(get_current_value(prefix, obj.name) .. index)
            end,
            int = function(index) return
                Append('%lld', get_current_value(prefix, obj.name) .. index)
            end,
            real = function(index) return
                Append('%e', get_current_value(prefix, obj.name) .. index)
            end,
            bool = function(index) return
                Append('%s', IfThenElse(get_current_value(prefix, obj.name) .. index, Str('true'), Str('false')))
            end,
            object = function() return
                Append('{') ..
                gen_pack(obj.children, get_new_prefix(prefix, obj.name)) ..
                Append('}')
            end,
            any = function() return CodeBlock {
                Switch(get_current_value(prefix, obj.name) .. '.type'),
                CodeBlock {
                    Case('JSON_OBJ_NULL', Append('null')),
                    Case('JSON_OBJ_INTEGER', Append('%lld', get_current_value(prefix, obj.name) .. '.integer')),
                    Case('JSON_OBJ_REAL', Append('%e', get_current_value(prefix, obj.name) .. '.real')),
                    Case('JSON_OBJ_BOOL', Append('%s', IfThenElse(get_current_value(prefix, obj.name) .. '.boolean', Str('true'), Str('false')))),
                    Case('JSON_OBJ_STRING', AppendString(get_current_value(prefix, obj.name) .. '.string_')),
                    'default: break;'
                }
            }
            end,
            _ = function() error("whoops") end
        }


        local array_wrap = function() return fn('') end
        if obj.length == -1 then
            local length = get_current_length(prefix, 'length_of_' .. obj.name)
            array_wrap = function()
                return CodeBlock {
                    Append('['),
                    Declare('int', 'k'),
                    If(length .. '> 0'),
                    CodeBlock {
                        fn('[0]'),
                        For('k = 1', 'k < '  .. length, '++k'),
                        CodeBlock {
                            Append(','),
                            fn('[k]')
                        },
                    },
                    Append(']')
                }
            end
        end

        if obj.is_optional then
            res[#res+1] = If(Isset(prefix, obj.name)) ..
            CodeBlock {
                Append('%s' .. key .. ':', maybe_comma),
                array_wrap()
            }
        else
            res[#res+1] = Append('%s' .. key .. ':', maybe_comma)
            res[#res+1] = array_wrap()
        end

        is_first = false
        obj = obj.next
    end

    return table.concat(res)
end

local function gen_validate(obj, prefix)
    local res = { }

    while obj do
        if not obj.is_optional then
            res[#res+1] = If(Not(Isset(prefix, obj.name))) ..
                indent(Goto('failure'))
        end

        if(obj.type == 'object') then
            if(obj.is_optional) then
                res[#res+1] = If(Isset(prefix, obj.name)) ..
                    CodeBlock {
                        gen_validate(obj.children, get_new_prefix(prefix, obj.name))
                    }
            else
                res[#res+1] = gen_validate(obj.children, get_new_prefix(prefix, obj.name))
            end
        end

        obj = obj.next
    end

    return table.concat(res)
end

local function gen_expect()
    return 'static int ' .. JSON_NAME .. '_expect(struct jslex* lexer, enum jslex_token_type type)\n' ..
    CodeBlock {
        'struct jslex_token* tok = jslex_next_token(lexer);\n',
        'if(!tok)\n',
        '    return 0;\n',
        '\n',
        'return tok->type == type;\n'
    } .. '\n'
end

local function gen_match_primitive(fn_name, tok_name)
    return 'static int ' .. JSON_NAME .. '_' .. fn_name .. '(struct jslex* lexer)\n' ..
    CodeBlock {
        If(Not(JSON_NAME .. '_expect(lexer, ' .. tok_name .. ')')),
        indent('return 0;\n'),
        'jslex_accept_token(lexer);\n',
        'return 1;\n'
    } .. '\n'
end

local function gen_match_key()
    return 'static int ' .. JSON_NAME .. '_key(struct jslex* lexer, const char* key)\n' ..
    CodeBlock {
        'struct jslex_token* tok = jslex_next_token(lexer);\n',
        'if(!tok)\n',
        '    return 0;\n',
        '\n',
        'if(tok->type != JSLEX_STRING)\n',
        '    return 0;\n',
        '\n',
        'if(0 != strcmp(key, tok->value.str))\n',
        '    return 0;\n',
        '\n',
        'jslex_accept_token(lexer);\n',
        'return 1;\n'
    } .. '\n'
end

local function gen_match_junk_array()
    return 'static int ' .. JSON_NAME .. '_junk_array(struct jslex* lexer)\n' ..
    CodeBlock {
        'return ', JSON_NAME, '_lbracket(lexer) && (', JSON_NAME, '_rbracket(lexer) || (',
            JSON_NAME, '_junk_values(lexer) && ', JSON_NAME, '_rbracket(lexer)));\n'
    } .. '\n'
end

local function gen_match_junk_values()
    return 'static int ' .. JSON_NAME .. '_junk_values(struct jslex* lexer)\n' ..
    CodeBlock {
        'return ', JSON_NAME, '_junk_value(lexer) && (', JSON_NAME, '_comma(lexer) ? ',
            JSON_NAME, '_junk_values(lexer) : 1);\n'
    } .. '\n'
end

local function gen_match_junk_value()
    return 'static int ' .. JSON_NAME .. '_junk_object(struct jslex* lexer);\n\n' ..
    'static int ' .. JSON_NAME .. '_junk_array(struct jslex* lexer);\n\n' ..
    'static int ' .. JSON_NAME .. '_junk_value(struct jslex* lexer)\n' ..
    CodeBlock {
        'return ', JSON_NAME, '_junk_integer(lexer)\n',
        '    || ', JSON_NAME, '_junk_real(lexer)\n',
        '    || ', JSON_NAME, '_junk_literal(lexer)\n',
        '    || ', JSON_NAME, '_junk_string(lexer)\n',
        '    || ', JSON_NAME, '_junk_array(lexer)\n',
        '    || ', JSON_NAME, '_junk_object(lexer);\n'
    } .. '\n'
end

local function gen_match_junk_member()
    return 'static int ' .. JSON_NAME .. '_junk_member(struct jslex* lexer)\n' ..
    CodeBlock {
        'return ', JSON_NAME, '_junk_string(lexer) && ', JSON_NAME, '_colon(lexer) && ',
            JSON_NAME, '_junk_value(lexer);\n'
    } .. '\n'
end

local function gen_match_junk_members()
    return 'static int ' .. JSON_NAME .. '_junk_members(struct jslex* lexer)\n' ..
    CodeBlock {
        'return ', JSON_NAME, '_junk_member(lexer) && (', JSON_NAME, '_comma(lexer) ? ',
            JSON_NAME, '_junk_members(lexer) : 1);\n'
    } .. '\n'
end

local function gen_match_junk_object()
    return 'static int ' .. JSON_NAME .. '_junk_object(struct jslex* lexer)\n' ..
    CodeBlock {
        'return ', JSON_NAME, '_lbrace(lexer) && (', JSON_NAME, '_rbrace(lexer) || (',
            JSON_NAME, '_junk_members(lexer) && ', JSON_NAME, '_rbrace(lexer)));\n'
    } .. '\n'
end

local function gen_unpack_any_value()
    return 'static int ' .. JSON_NAME .. '_any_value(struct json_obj_any* any, struct jslex* lexer)\n' ..
    CodeBlock {
        'return ', JSON_NAME, '_any_integer(any, lexer)\n',
        '    || ', JSON_NAME, '_any_real(any, lexer)\n',
        '    || ', JSON_NAME, '_any_bool(any, lexer)\n',
        '    || ', JSON_NAME, '_any_string(any, lexer);\n'
    } .. '\n'
end

local function gen_unpack_any_type(typename, token, json_type, assignment)
    local res = {
        'static int ', JSON_NAME, '_any_', typename, '(struct json_obj_any* obj, struct jslex* lexer)\n',
        CodeBlock {
            'struct jslex_token* tok = jslex_next_token(lexer);\n',
            'if(!tok)\n',
            '    return 0;\n',
            '\n',
            'if(tok->type != ', token, ')\n',
            '    return 0;\n',
            '\n',
            'obj->type = ', json_type, ';\n',
            assignment,
            '\n',
            'jslex_accept_token(lexer);\n',
            'return 1;\n'
        }, '\n'
    }
    return table.concat(res)
end

local function gen_unpack_primitive_tokens()
    local res = { }

    res[#res+1] = gen_expect()
    res[#res+1] = gen_match_key()
    res[#res+1] = gen_match_primitive('lbracket', 'JSLEX_LBRACKET')
    res[#res+1] = gen_match_primitive('rbracket', 'JSLEX_RBRACKET')
    res[#res+1] = gen_match_primitive('lbrace', 'JSLEX_LBRACE')
    res[#res+1] = gen_match_primitive('rbrace', 'JSLEX_RBRACE')
    res[#res+1] = gen_match_primitive('comma', 'JSLEX_COMMA')
    res[#res+1] = gen_match_primitive('colon', 'JSLEX_COLON')
    res[#res+1] = gen_match_primitive('junk_integer', 'JSLEX_INTEGER')
    res[#res+1] = gen_match_primitive('junk_real', 'JSLEX_REAL')
    res[#res+1] = gen_match_primitive('junk_string', 'JSLEX_STRING')
    res[#res+1] = gen_match_primitive('junk_literal', 'JSLEX_LITERAL')
    res[#res+1] = gen_match_junk_value()
    res[#res+1] = gen_match_junk_values()
    res[#res+1] = gen_match_junk_array()
    res[#res+1] = gen_match_junk_member()
    res[#res+1] = gen_match_junk_members()
    res[#res+1] = gen_match_junk_object()
    res[#res+1] = gen_unpack_any_type('integer', 'JSLEX_INTEGER', 'JSON_OBJ_INTEGER',
        'obj->integer = tok->value.integer;')
    res[#res+1] = gen_unpack_any_type('real', 'JSLEX_REAL', 'JSON_OBJ_REAL',
        'obj->real = tok->value.real;')
    res[#res+1] = gen_unpack_any_type('bool', 'JSLEX_LITERAL', 'JSON_OBJ_BOOL',
        'obj->boolean = (strcmp(tok->value.str, "true") == 0);')
    res[#res+1] = gen_unpack_any_type('string', 'JSLEX_STRING', 'JSON_OBJ_STRING',
        'obj->string_ = strdup(tok->value.str);')
    res[#res+1] = gen_unpack_any_value()

    return table.concat(res)
end

local function gen_unpack_object_members(obj, prefix)
    local values = { }
    local child = obj.children

    while child do
        values[#values+1] = myconcat('__', JSON_NAME, prefix, obj.name, child.name) .. '(dst, lexer)'
        child = child.next
    end

    values[#values+1] = JSON_NAME .. '_junk_member(lexer)'

    return 'return ' .. table.concat(values, '\n    || ') .. ';\n'
end

local function gen_unpack_object_value(obj, prefix)
    local full_prefix = myconcat('__', JSON_NAME, prefix, obj.name)
    return table.concat{
        'static int ', full_prefix, '_member(struct ', JSON_NAME, '* dst, struct jslex* lexer)\n',
        CodeBlock {
            gen_unpack_object_members(obj, prefix)
        },
        '\n',
        'static int ', full_prefix, '_members(struct ', JSON_NAME, '* dst, struct jslex* lexer)\n',
        CodeBlock {
            'return ', full_prefix, '_member(dst, lexer) && (',
            JSON_NAME, '_comma(lexer) ? ', full_prefix, '_members(dst, lexer) : 1)\n;'
        },
        '\n',
        'static int ', full_prefix, '_value(struct ', JSON_NAME, '* dst, struct jslex* lexer)\n',
        CodeBlock {
            'return ', JSON_NAME, '_lbrace(lexer) && (', JSON_NAME, '_rbrace(lexer) || (',
                full_prefix, '_members(dst, lexer) && ', JSON_NAME, '_rbrace(lexer)));\n'
        },
        '\n',
    }
end

local function gen_unpack_object_object(obj, prefix)
    local full_prefix = myconcat('__', JSON_NAME, prefix, obj.name)
    local isset_path = myconcat('.', prefix, 'is_set_' .. obj.name)
    return table.concat{
        'static int ', full_prefix, '(struct ', JSON_NAME, '* dst, struct jslex* lexer)\n',
        CodeBlock {
            'int res =  ', JSON_NAME, '_key(lexer, "', obj.name,'") && ',
            JSON_NAME, '_colon(lexer) && ', full_prefix, '_value(dst, lexer);\n',
            'dst->', isset_path, ' = res;\n',
            'return res;\n'
        },
        '\n'
    }
end

local function gen_unpack_object(obj, prefix)
    local res = { }

    res[#res+1] = gen_unpack_object_functions(obj.children, flatten{prefix, obj.name})
    res[#res+1] = gen_unpack_object_value(obj, prefix)
    res[#res+1] = gen_unpack_object_object(obj, prefix)

    return table.concat(res)
end

local function gen_unpack_any(obj, prefix)
    local full_path = myconcat('__', JSON_NAME, prefix, obj.name)
    local isset_path = myconcat('.', prefix, 'is_set_' .. obj.name)
    local value_path = myconcat('.', prefix, obj.name)
    local res = {
        'static int ', full_path, '(struct ', JSON_NAME, '* dst, struct jslex* lexer)\n',
        CodeBlock {
            'if(!', JSON_NAME, '_key(lexer, "', obj.name,'"))\n',
            '    return 0;\n',
            '\n',
            'if(!', JSON_NAME, '_colon(lexer))\n',
            '    return 0;\n',
            '\n',
            'if(!', JSON_NAME, '_any_value(&dst->', value_path, ', lexer))\n',
            '    return 0;\n',
            '\n',
            'dst->', isset_path, ' = 1;\n',
            'return 1;\n'
        }, '\n'
    }
    return table.concat(res)
end

local function type_to_token(tp)
    return match(tp) {
        int = 'JSLEX_INTEGER',
        real = 'JSLEX_REAL',
        string = 'JSLEX_STRING',
        bool = 'JSLEX_LITERAL',
        _ = 'ERROR'
    }
end

local function gen_assign_integer(obj, prefix)
    return 'dst->' .. myconcat('.', prefix, obj.name) .. ' = tok->value.integer;\n'
end

local function gen_assign_real(obj, prefix)
    return 'dst->' .. myconcat('.', prefix, obj.name) .. ' = tok->value.real;\n'
end

local function gen_assign_string(obj, prefix)
    return 'dst->' .. myconcat('.', prefix, obj.name) .. ' = strdup(tok->value.str);\n'
end

local function gen_assign_bool(obj, prefix)
    return 'dst->' .. myconcat('.', prefix, obj.name) .. ' = (strcmp(tok->value.str, "true") == 0);\n'
end

local function gen_assign_simple_value(obj, prefix)
    return match(obj.type) {
        int = gen_assign_integer,
        real = gen_assign_real,
        string = gen_assign_string,
        bool = gen_assign_bool
    } (obj, prefix)
end

local function gen_unpack_simple(obj, prefix)
    local res = {
        'static int ', myconcat('__', JSON_NAME, prefix, obj.name), '_value(struct ', JSON_NAME, '* dst, struct jslex* lexer)\n',
        CodeBlock {
            'struct jslex_token* tok = jslex_next_token(lexer);\n',
            'if(!tok)\n',
            '    return 0;\n',
            '\n',
            'if(tok->type != ', type_to_token(obj.type), ')\n',
            '    return 0;\n',
            '\n',
            gen_assign_simple_value(obj, prefix),
            'dst->', myconcat('.', prefix, 'is_set_' .. obj.name), ' = 1;\n',
            '\n',
            'jslex_accept_token(lexer);\n',
            'return 1;\n'
        },
        '\n',
        'static int ', myconcat('__', JSON_NAME, prefix, obj.name), '(struct ', JSON_NAME, '* dst, struct jslex* lexer)\n',
        CodeBlock {
            'return ', JSON_NAME, '_key(lexer, "', obj.name,'") && ',
            JSON_NAME, '_colon(lexer) && ',
             myconcat('__', JSON_NAME, prefix, obj.name), '_value(dst, lexer);\n'
        }, '\n'
    }
    return table.concat(res)
end

local function gen_grow_and_append_array(obj, prefix)
    local full_path = myconcat('__', JSON_NAME, prefix, obj.name)
    local reserved_size = myconcat('.', prefix, 'reserved_size_of_' .. obj.name)
    local length = myconcat('.', prefix, 'length_of_' .. obj.name)
    local value_path = myconcat('.', prefix, obj.name)

    local res = {
        'static int ', full_path, '_grow(struct ', JSON_NAME, '* dst, size_t new_size)\n',
        CodeBlock {
            'if(new_size <= dst->', reserved_size, ')\n',
            '    return 0;\n',
            'else\n',
            '    dst->', reserved_size, ' = new_size * 2;\n',
            '\n',
            'dst->', value_path, ' = realloc(dst->', value_path, ', sizeof(', obj.ctype, ') * dst->', reserved_size, ');\n',
            'if(!dst->', value_path, ')\n',
            '    return -1;\n',
            '\n',
            'return 0;\n'
        },
        '\n',
        'static int ', full_path, '_append(struct ', JSON_NAME, '* dst, ', obj.ctype, ' elem)\n',
        CodeBlock {
            'if(', full_path, '_grow(dst, dst->', length, ' + 1) < 0)\n',
            '    return -1;\n',
            '\n',
            'dst->', value_path, '[dst->', length, '++] = elem;\n',
            '\n',
            'return 0;'
        },
        '\n'
    }

    return table.concat(res)
end

local function gen_append_integer(obj, prefix)
    return 'if(' .. myconcat('__', JSON_NAME, prefix, obj.name) .. '_append(dst, tok->value.integer) < 0)\n' ..
            '    return 0;\n'
end

local function gen_append_real(obj, prefix)
    return 'if(' .. myconcat('__', JSON_NAME, prefix, obj.name) .. '_append(dst, tok->value.real) < 0)\n' ..
            '    return 0;\n'
end

local function gen_append_string(obj, prefix)
    local res = {
        'char* copy = strdup(tok->value.str);\n',
        'if(!copy)\n',
        '    return 0;\n',
        '\n',
        'if(', myconcat('__', JSON_NAME, prefix, obj.name) .. '_append(dst, copy) < 0)\n',
        CodeBlock {
            'free(copy);\n',
            'return 0;\n',
        },
    }
    return table.concat(res)
end

local function gen_append_bool(obj, prefix)
    return 'if(' .. myconcat('__', JSON_NAME, prefix, obj.name) .. '_append(dst, strcmp(tok->value.str, "true") == 0) < 0)\n' ..
            '    return 0;\n'
end

local function gen_append_array_value(obj, prefix)
    return match(obj.type) {
        int = gen_append_integer,
        real = gen_append_real,
        string = gen_append_string,
        bool = gen_append_bool
    } (obj, prefix)
end

local function gen_unpack_array_values(obj, prefix)
    local full_path = myconcat('__', JSON_NAME, prefix, obj.name)
    local isset_path = myconcat('.', prefix, 'is_set_' .. obj.name)

    local res = {
        'static int ', full_path, '_value(struct ', JSON_NAME, '* dst, struct jslex* lexer)\n',
        CodeBlock {
            'struct jslex_token* tok = jslex_next_token(lexer);\n',
            'if(!tok)\n',
            '    return 0;\n',
            '\n',
            'if(tok->type != ', type_to_token(obj.type), ')\n',
            '    return 0;\n',
            '\n',
            gen_append_array_value(obj, prefix),
            '\n',
            'jslex_accept_token(lexer);\n',
            'return 1;\n'
        },
        '\n',
        'static int ', full_path, '_values(struct ', JSON_NAME, '* dst, struct jslex* lexer)\n',
        CodeBlock {
            'return ', full_path, '_value(dst, lexer) && (',
            JSON_NAME, '_comma(lexer) ? ', full_path, '_values(dst, lexer) : 1);\n'
        },
        '\n',
        'static int ', full_path, '_array(struct ', JSON_NAME, '* dst, struct jslex* lexer)\n',
        CodeBlock {
            'int res = ', JSON_NAME, '_lbracket(lexer) && (', JSON_NAME, '_rbracket(lexer) || (',
            full_path, '_values(dst, lexer) && ', JSON_NAME, '_rbracket(lexer)));\n',
            'dst->', isset_path, ' = res;\n',
            'return res;\n'
        },
        '\n',
        'static int ', full_path, '(struct ', JSON_NAME, '* dst, struct jslex* lexer)\n',
        CodeBlock {
            'return ', JSON_NAME, '_key(lexer, "', obj.name,'") && ',
            JSON_NAME, '_colon(lexer) && ', full_path, '_array(dst, lexer);\n'
        }, '\n'
    }

    return table.concat(res)
end

local function gen_unpack_array(obj, prefix)
    local res = {
        gen_grow_and_append_array(obj, prefix),
        gen_unpack_array_values(obj, prefix)
    }
    return table.concat(res)
end

function gen_unpack_object_functions(obj, prefix)
    local res = { }

    while obj do
        res[#res+1] = match(obj.type) {
            object = gen_unpack_object,
            any = gen_unpack_any,
            _ = function(...)
                if(obj.length == 1) then
                    return gen_unpack_simple(...)
                elseif(obj.length == -1) then
                    return gen_unpack_array(...)
                end
            end

        } (obj, prefix)

        obj = obj.next
    end

    return table.concat(res)
end

local function gen_unpack_functions(obj)
    local res = { }

    res[#res+1] = gen_unpack_primitive_tokens()
    res[#res+1] = gen_unpack_object_functions(obj, {})
    res[#res+1] = gen_unpack_object_value({ name={}, children=obj }, {})

    return table.concat(res)
end

local function gen_cleanup_dynamic_string_array(prefix, obj)
    local full_path = get_current_value(prefix, obj.name)
    local length = get_current_length(prefix, 'length_of_' .. obj.name)

    local res = {
        If(Isset(prefix, obj.name)),
        CodeBlock {
            CodeBlock {
                'int i;\n',
                'for(i = 0; i < ', length, '; ++i)\n',
                '    free(', full_path, '[i]);\n'
            },
            'free(' .. full_path .. ");\n"
        }
    }

    return table.concat(res)
end

local function gen_cleanup_dynamic_array(prefix, obj)
    local res = {
        If(Isset(prefix, obj.name)),
        indent('free(' .. get_current_value(prefix, obj.name) .. ");\n")
    }

    return table.concat(res)
end

local function gen_cleanup(obj, prefix)
    local res = { }

    while obj do
        match(obj.type) {
            object = function()
                res[#res+1] = If(Isset(prefix, obj.name)) ..
                    CodeBlock {
                        gen_cleanup(obj.children, get_new_prefix(prefix, obj.name))
                    }
            end,
            string = function()
                if(obj.length == -1) then
                    res[#res+1] = gen_cleanup_dynamic_string_array(prefix, obj)
                else
                    res[#res+1] = If(Isset(prefix, obj.name)) ..
                        indent("free(" .. get_current_value(prefix, obj.name) .. ");\n")
                end
            end,
            any = function()
                res[#res+1] = If(And(Isset(prefix, obj.name),
                                     Eq(get_current_value(prefix, obj.name) .. ".type",
                                        'JSON_OBJ_STRING'))) ..
                    indent("free(" .. get_current_value(prefix, obj.name) .. ".string_);\n")
            end,
            _ = function()
                if(obj.length == -1) then
                    res[#res+1] = gen_cleanup_dynamic_array(prefix, obj)
                end
            end
        } ()

        obj = obj.next
    end

    return table.concat(res)
end

local output = {
[[#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jslex.h"

]],
'#include "', JSON_NAME, '.h"', [[


]], gen_unpack_functions(JSON_ROOT),
"void ", JSON_NAME, "_cleanup(struct ", JSON_NAME, "* obj)\n",
    CodeBlock {
        gen_cleanup(JSON_ROOT)
    },
"\n",
"ssize_t ", JSON_NAME, "_unpack(struct ", JSON_NAME, [[* obj, const char* data)
{
    memset(obj, 0, sizeof(*obj));

    struct jslex lexer;
    if(jslex_init(&lexer, data) < 0)
        return -1;

    if(!]], JSON_NAME, [[_value(obj, &lexer))
        goto failure;

]], indent(gen_validate(JSON_ROOT)), [[

    jslex_cleanup(&lexer);
    return lexer.next_pos - data;

failure:
    ]], JSON_NAME, [[_cleanup(obj);
    jslex_cleanup(&lexer);
    return -1;
}

char* ]], JSON_NAME, [[_pack(const struct ]], JSON_NAME, [[* obj)
{
    size_t size = 4096;
    char* buffer = malloc(size);
    if(!buffer)
        return NULL;
    int comma = 0;
    int i = 0;
    char* str;
]], indent(Append('{') ..
    gen_pack(JSON_ROOT) ..
    Append('}')), [[
    return buffer;

failure:
    if(str)
        free(str);
    free(buffer);
    return NULL;
}
]]
}

print(table.concat(output))

