-- local lsl_pointer = require('liblsl_pointer')

-- local test_str = lsl_pointer:new()

-- local addr = test_str:test_set_str("lsl-test")
-- print(string.format("lsl-debug: addr in lua: %x", addr))
-- print(test_str:test_get_str())
-- print(test_str)
--

local function get_self_statm()
    local fd = io.open('/proc/self/statm')
    local lines = {}

    for line in fd:lines() do
        table.insert(lines, line)
    end

    fd:close()

    local content = table.concat(lines, '\n')

    -- print('statm: ' .. content)

    return content
end


local function get_second_field(statm)
    local cnt = 1
    for data in string.gmatch(statm, '%S+') do
        if cnt == 2 then
            return data
        end

        cnt = cnt + 1
    end
end


local shared_dict = require('libshared_dict')


local function do_test(sm_obj)
    print('base addr: ' .. sm_obj:get_base_addr_str())
    get_self_statm()

    sm_obj:load_pages()
    local statm = get_self_statm()

    local statis = {}
    for i = 0, 9, 1 do
        sm_obj:release_region(i)
        local last = get_second_field(statm)
        statm = get_self_statm()

        local now = get_second_field(statm)
        table.insert(statis, tonumber(last) - tonumber(now))
    end

    for idx, diff in ipairs(statis) do
        print(string.format('[%02d] diff %d', idx, diff))
    end
end

local sm_obj = shared_dict.new('0')
do_test(sm_obj)

print('\n==================\n')
local sm_a = shared_dict.new(sm_obj:get_base_addr_str())
do_test(sm_a)

io.read()

sm_a:destory()
