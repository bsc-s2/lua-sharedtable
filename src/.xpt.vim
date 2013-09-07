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

call XPTemplate( 'st',  'size_t' )
call XPTemplate( 'sst',  'ssize_t' )

call XPTemplate('null', 'NULL')
call XPTemplate('nil', 'NULL')
call XPTemplate('=n', ' == NULL')
call XPTemplate('!n', ' != NULL')
call XPTemplate('=0', ' == 0')
call XPTemplate('!0', ' != 0')
call XPTemplate('=1', ' == 1')
call XPTemplate('!1', ' != 1')

call XPTemplate( 'so',  'sizeof(`cursor^)' )

call XPTemplate( '.ok',     'ST_OK' )
call XPTemplate( '.err',    'ST_ERR' )

call XPTemplate( '.',     'st_' )
call XPTemplate( '.as',   'st_assert' )
call XPTemplate( '.mu',   'st_must' )

call XPTemplate( '.e',    'ST_' )
call XPTemplate( '.at',   'st_atomic_' )
call XPTemplate( '.gf',   'st_gf_' )
call XPTemplate( '.lq',   'st_lqueue_' )
call XPTemplate( '.lfq',  'st_lfqueue_' )
call XPTemplate( '.mt',   'st_matrix_' )

" unittest assertion
call XPTemplate( '.ua',   'st_ut_' )
call XPTemplate( '.ut',   'st_util_' )

call XPTemplate( '.sl',   'st_sl_' )
call XPTemplate( '.sle',  'st_sle_' )
call XPTemplate( '.tm',   'st_timer_' )
call XPTemplate( '.aio',  'st_aio_' )
call XPTemplate( '.ep',   'st_epoll_' )
" call XPTemplate( '.en',   'st_eng_' )

call XPTemplate( '.net',   'st_net_' )
call XPTemplate( '.ip4',   'st_net_ip4_' )
call XPTemplate( '.tcp',   'st_net_ip4_tcp_' )

call XPTemplate( '.bt', 'st_btu64_' )
call XPTemplate( '.btkv', 'st_btu64_kv_' )
call XPTemplate( '.btn', 'st_btu64_node_' )
call XPTemplate( '.bti', 'st_btu64_iter_' )
call XPTemplate( '.bts', 'st_btu64_search_rst_t' )

call XPTemplate( 'aeq', 'st_eq(`ST_OK^, `rc^, `""^);' )
call XPTemplate( 'ane', 'st_ne(`ST_OK^, `rc^, `""^);' )

call XPTemplate('modeline', '// vim'.':ts=8:sw=4:et:fdl=0')
