" inoremap <expr> - getline( "." )[ col( "." ) - 2 ] =~ '\V\w' ? "_" : "-"

call XPTemplate( 'i8',  'int8_t' )
call XPTemplate( 'i16', 'int16_t' )
call XPTemplate( 'i32', 'int32_t' )
call XPTemplate( 'i64', 'int64_t' )
call XPTemplate( 'ip',  'intptr_t' )

call XPTemplate( 'u8',  'uint8_t' )
call XPTemplate( 'u16', 'uint16_t' )
call XPTemplate( 'u32', 'uint32_t' )
call XPTemplate( 'u64', 'uint64_t' )
call XPTemplate( 'up',  'uintptr_t' )

call XPTemplate( '.',     'xpj_' )

call XPTemplate( '.e',    'xpj_e_' )
call XPTemplate( '.ut',   'xpj_util_' )

call XPTemplate( '.sl',   'xpj_sl_' )
call XPTemplate( '.sle',  'xpj_sle_' )
call XPTemplate( '.tm',   'xpj_timer_' )
call XPTemplate( '.aio',  'xpj_aio_' )
call XPTemplate( '.ep',   'xpj_epoll_' )

call XPTemplate( '.net',   'xpj_net_' )
call XPTemplate( '.ip4',   'xpj_net_ip4_' )
call XPTemplate( '.tcp',   'xpj_net_ip4_tcp_' )

call XPTemplate( '.bt', 'xpj_btu64_' )
call XPTemplate( '.btkv', 'xpj_btu64_kv_' )
call XPTemplate( '.btn', 'xpj_btu64_node_' )
call XPTemplate( '.bti', 'xpj_btu64_iter_' )
call XPTemplate( '.bts', 'xpj_btu64_search_rst_t' )

call XPTemplate( 'aeq', 'ass_eq( `int^, `XPJ_E_OK^, `rc^, "`^"`, `a?^ );' )
call XPTemplate( 'ane', 'ass_ne( `int^, `XPJ_E_OK^, `rc^, "`^"`, `a?^ );' )
