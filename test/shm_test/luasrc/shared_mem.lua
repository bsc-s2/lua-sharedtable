local shared_dict = require('libshared_dict')


local _M = {
    _VERSION = 1.0,
    shared_mem = nil, -- module cache for shared_mem userdata to avoid gc
}


local meta_table = { __index = _M }


local function make_shared_mem(is_inited, addr, shared_dict)
    local obj = {
        _VERSION    = 1.0,
        shared_dict = shared_dict,
        is_inited   = is_inited,
        addr        = addr,
    }

    return setmetatable(obj, meta_table)
end


function _M.new()
    if _M.shared_mem == nil then
        local sm_obj = shared_dict.new('0')

        local base_addr = sm_obj:get_base_addr_str()
        ngx.log(ngx.INFO,
                'lsl-debug: init shared_mem addr: ' .. base_addr)

        _M.shared_mem = make_shared_mem(true, base_addr, sm_obj)
    else
        ngx.log(ngx.INFO, 'lsl-debug: use shared_mem cache')
    end

    return _M.shared_mem, nil, nil
end


function _M.force_load_pages(self)
    ngx.log(ngx.INFO, 'lsl-debug: force_load_pages is called')
    local sm_obj = self.shared_dict

    ngx.log(ngx.INFO, 'lsl-debug: base addr : ' .. sm_obj:get_base_addr_str())
    sm_obj:load_pages()
end


function _M.release_region(self, index)
    local sm_obj = self.shared_dict

    sm_obj:release_region(index)
end


return _M
