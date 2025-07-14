-- util : utility functions

local m = {}

function m.split(str, delimiter)
    local result = {}
    local pattern = "(.-)" .. delimiter .. "()"
    local nb = 0
    local lastPos
    for part, pos in string.gmatch(str .. delimiter, pattern) do
        nb = nb + 1
        result[nb] = part
        lastPos = pos
    end
    result[nb] = string.sub(str, lastPos)
    if result[nb] == "" then
        result[nb] = nil
    end
    return result
end

function m.trim(str)
    return str:match("^%s*(.-)%s*$")
end

function m.starts_with(str, prefix)
    return string.sub(str, 1, string.len(prefix)) == prefix
end

function m.ends_with(str, suffix)
    return suffix == "" or string.sub(str, -string.len(suffix)) == suffix
end

function m.table_contains(table, element)
    for _, value in pairs(table) do
        if value == element then
            return true
        end
    end
    return false
end

function m.merge_tables(t1, t2)
    local result = {}
    for k, v in pairs(t1) do
        result[k] = v
    end
    for k, v in pairs(t2) do
        result[k] = v
    end
    return result
end

return m
