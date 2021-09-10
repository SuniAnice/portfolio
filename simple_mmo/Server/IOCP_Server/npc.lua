myid = -1


function set_object_info(id)


myid = id;
end

function player_is_near( p_id )
	p_x = API_get_x(p_id)
	p_y = API_get_y(p_id)
	m_x = API_get_x(myid)
	m_y = API_get_y(myid)
	
	if (math.abs(p_y - m_y) < 5) then
		if (math.abs(p_x - m_x) < 5) then
			API_ATTACK_PLAYER(p_id, myid);
		end
	end
end
