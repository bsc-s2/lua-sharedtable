local shared_mem  = require('shared_mem')


local _M = { _VERSION = 1.0 }


local index = 0


local function test_release()
    local sharedm_obj = ngx.ctx.sharedm_obj
    sharedm_obj:release_region(index)
    index = (index + 1) % 10

    ngx.say(ngx.var.pid)
    ngx.eof()
    ngx.exit(ngx.HTTP_OK)
end


local function test_report()
    local fd = io.open('/proc/self/statm')
    local lines = {}

    for line in fd:lines() do
        table.insert(lines, line)
    end

    fd:close()

    local content = table.concat(lines, '\n')
    ngx.log(ngx.INFO, 'lsl-debug: report: ' .. content)

    ngx.say(ngx.var.pid .. ': ' .. content)
    ngx.eof()
    ngx.exit(ngx.HTTP_OK)
end


local function force_load_pages()
    local sharedm_obj = ngx.ctx.sharedm_obj

    sharedm_obj:force_load_pages()

    ngx.say(ngx.var.pid .. ': force loaded');
    ngx.eof()
    ngx.exit(ngx.HTTP_OK)
end


function _M.rewrite()
    local sharedm_obj, err, errmsg = shared_mem.new(false)
    if err ~= nil then
        ngx.log(ngx.ERR,
                'lsl-debug: failed to init shared memory: ', err, errmsg)
    end

    ngx.ctx.sharedm_obj = sharedm_obj

    ngx.log(ngx.INFO, 'lsl-debug: shared_mem: ' .. sharedm_obj.addr)

    local raw_uri = ngx.var.request_uri
    ngx.log(ngx.INFO, 'lsl-debug: request received! ' .. raw_uri)

    if raw_uri == '/release' then
        test_release()
    elseif raw_uri == '/load' then
        force_load_pages()
    else
        test_report()
    end
end


return _M
