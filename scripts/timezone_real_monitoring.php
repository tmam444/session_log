<?php
header("Content-Type: text/plain ; charset=utf-8");

//session_start();
//session_register("sessTraffic_Out");
//session_register("sessPacket_Out");
//session_register("sessTraffic_Int");
//session_register("sessPacket_Int");
//session_register("sessCnt");

//************************************//
include ("../dbconn.php");
include ("../function_color.php");
include ("../CMS/socket_lib.php");
//************************************//
// $ip is query string
/*
{

"realTrafficIn":[{"data":[1442812219,56],"label":"P2P","color":"red"}],
"realTrafficOut":[{"data":[1442812219,100],"label":"Webservice","color":"red"}],

"hddData":[{"data":20351000000,"label":"사용량","color":"#204BA0"},
{"data":979649000000,"label":"총용량","color":"#A02020"}],
"hddStatus":{"usage":20351000000,"total":344324234,"percentage":"2%"},

"cpuData":[{"data":[1442812219,3],"label":"CPU","color":"#00FF00"}],
"cpuStatus":{"usage":20351000000,"total":344324234,"percentage":"2%"},

"memoryData":[{"data":[1442812219,64],"label":"Memory","color":"#00FF00"}], //
실제 그래프
"memStatus":{"usage":20351000000,"total":344324234,"percentage":"2%"}, // 상태표시

"portStatus":[{"number":"PORT1","lineStatus":2}]

}
*/

/* -------------------------------------------------------------------------------------------------------------------------*/
/* -------------- 정책별 모니터링 시 장비의 선택 클래스
* 알아오기------------------------------------------------------*/
/* -------------------------------------------------------------------------------------------------------------------------
class_i_name=
class_i_cid=
class_e_name=
class_e_cid=
class_cid_arr=

$class_i_cid="18,19,20";
$class_i_name="미분류,제어,차단";
$class_e_cid="18,19,20";
$class_e_name="미분류,제어,차단";
$class_cid_arr="18,19,20";
*/

/*
$DEVICE_CLASS_INFO_CONF = "/usr/lib/qosd/conf/device_policy_monitoring_" . $ip;
@unlink($DEVICE_CLASS_INFO_CONF);

system("wget -t 4 -O " . $DEVICE_CLASS_INFO_CONF . " http://" . $ip . "/CMS/cms_class_sel_info.php");

$mtime = time() + 10;
for ($i = $mtime; $i > time(); usleep(1)) {
if (file_exists($DEVICE_CLASS_INFO_CONF)) {

$class_i_name = trim(read_conf($DEVICE_CLASS_INFO_CONF, "class_i_name"));
$class_i_cid = trim(read_conf($DEVICE_CLASS_INFO_CONF, "class_i_cid"));
$class_e_name = trim(read_conf($DEVICE_CLASS_INFO_CONF, "class_e_name"));
$class_e_cid = trim(read_conf($DEVICE_CLASS_INFO_CONF, "class_e_cid"));
$class_cid_arr = trim(read_conf($DEVICE_CLASS_INFO_CONF, "class_cid_arr"));

$class_i_name_arr = explode(",", $class_i_name);
$class_i_cid_arr = explode(",", $class_i_cid);
$class_e_name_arr = explode(",", $class_e_name);
$class_e_cid_arr = explode(",", $class_e_cid);

break;
}
}

for ($i = 0; $i < sizeof($class_i_name_arr); $i++) {
$i_cid = $class_i_cid_arr[$i];
$cname_i_arr[$i_cid] = $class_i_name_arr[$i];
}

for ($i = 0; $i < sizeof($class_e_name_arr); $i++) {
$e_cid = $class_e_cid_arr[$i];
$cname_e_arr[$e_cid] = $class_e_name_arr[$i];
}
*/

// 2016-04-21 : Storage Server DB 정보로 적용시킴.
$weekName = strtoupper(date("D")) ;
$hh = date("H") . date("i") ;
$q_query  = "select qname_id, target from tblSchedule where week = '$weekName' and from_time <= $hh and to_time > $hh " ;
$q_result = mysql_query($q_query, $dbconn);
$q_row = mysql_fetch_row($q_result);
$qname_id = $q_row[0] ;

$sql = "select cid, cname from tblClass_int where hid_qid=$qname_id  and is_view='1' and is_drop != '1' order by hid_cid asc, cid asc " ;
$result = mysql_query($sql, $dbconn) ;
while( $row = mysql_fetch_array($result) ) {
	$int_cid = $row[0] ;
	$class_i_cid_arr[] = $row[0] ;
	$cname_i_arr[$int_cid] = $row[1] ;
}

$sql = "select cid, cname from tblClass_ext where hid_qid=$qname_id and is_view='1' and is_drop != '1' order by hid_cid asc, cid asc " ;
$result = mysql_query($sql, $dbconn) ;
while( $row = mysql_fetch_array($result) ) {
	$ext_cid = $row[0] ;
	$class_e_cid_arr[] = $row[0] ;
	$cname_e_arr[$ext_cid] = $row[1] ;
	//echo $ext_cid . "=>" . $row[1] ;
}

//echo $sql ;

if ( $dir == "i" ) {
	$class_cid_arr = implode(",", $class_i_cid_arr) ;
} else if ( $dir == "e" ) {
	$class_cid_arr = implode(",", $class_e_cid_arr) ;
}
$cname_i_arr[0] = "Total";
$cname_e_arr[0] = "Total";
$cname_i_arr[1] = "Total";
$cname_e_arr[1] = "Total";

//@unlink($DEVICE_CLASS_INFO_CONF) ;
//exit;

/* -------------------------------------------------------------------------------------------------------------------------*/
/* -------------- 정책별 모니터링 값
* 받아오기---------------------------------------------------------------------------*/
/* -------------------------------------------------------------------------------------------------------------------------*/

$band_unit_val = 0;
$cname_arr = unserialize(stripslashes($cname_arr_serialize));
$cname_arr[0] = "Total";
$json_cname_arr[0] = "Total";

/*
$multi_chk = trim(read_conf("/etc/rc.d/rc.device", "DEV_TYPE")) ;
if ($multi_chk == "q_bridge") {
$port_su = "4" ;
} else if ($multi_chk == "t_bridge") { // bridge
$port_su = "3" ;
} else if ($multi_chk == "m_bridge") {
$port_su = "2" ;
} else if ($multi_chk == "bridge") {
$port_su = "1" ;
} else {
$bridge_val = explode("_", $multi_chk) ;
$port_su = $bridge_val[0] ;
}
*/

if ( $port_flag == "ALL" ) $port_flag = 0 ;

function unit_func($val, $unit_val, $round_val) {
	if ($unit_val == "0") {
		$traffic_val = $val;
	} else if ($unit_val == "1") {
		$traffic_val = $val / 1000;
	} else if ($unit_val == "2") {
		$traffic_val = $val / 1000 / 1000;
	}
	return round($traffic_val, $round_val);
}

function return_func($dir, $int_traffic_val, $ext_traffic_val) {
	global $traffic_realtime, $traffic_unixtime, $cname_arr;
	global $realClass_session_i, $realClass_session_e, $band_unit_val, $band_graph_val;
	// 평균값 저장 위한 파일 명.

	$traffic_str = "";
	$json_label_val = "";
	$json_data_val = "";
	$json_data = "";

	/*
	{"data":[1442552912,75],"label":"P2P"}
	*/

	$i_cid_arr_val = 0;
	foreach ($int_traffic_val as $cid_key => $traffic_val) {// 내부방향 처리.
		$traffic_str .= unit_func($traffic_val, $band_unit_val, 1) . ",";
		//$json_data .= $cname_arr[$cid_key] . ":" . unit_func($traffic_val,
		// $band_unit_val, 0) . "," ;
		$json_data .= '{"data":[' . $traffic_unixtime . ',' . unit_func($traffic_val, $band_unit_val, 0) . '],"label":"' . $cname_arr[$cid_key] . '"},';

		$json_cname_arr[$i_cid_arr_val] = $cname_arr[$cid_key];
		$i_cid_arr_val++;
	}

	$e_cid_arr_val = 0;
	foreach ($ext_traffic_val as $cid_key => $traffic_val) {// 외부방향 처리.
		$traffic_str .= unit_func($traffic_val, $band_unit_val, 1) . ",";
		$json_data .= $cname_arr[$cid_key] . ":" . unit_func($traffic_val, $band_unit_val, 0) . ",";

		$json_cname_arr[$e_cid_arr_val] = $cname_arr[$cid_key];
		$e_cid_arr_val++;
	}

	$traffic_str_real = substr($traffic_str, 0, -1);

	if ($band_graph_val == "0") {// 평균 그래프

		if ($dir == "i") {
			$real_val_arr = explode("||", $realClass_session_i);
		} else {
			$real_val_arr = explode("||", $realClass_session_e);
		}

		if (sizeof($real_val_arr) < 10) {// 파일에 누적해서 10번의 데이터를 write한다.

			if ($dir == "i") {
				$realClass_session_i .= $traffic_str_real . "||";
			} else {
				$realClass_session_e .= $traffic_str_real . "||";
			}

		} else {

			$realClass_session_i = "";
			$realClass_session_e = "";
			$json_label_val = "";
			$json_data_val = "";
			$json_data = "";

			for ($i = 0; $i < sizeof($real_val_arr); $i++) {
				if ($i > 0) {
					if ($dir == "i") {
						$realClass_session_i .= $real_val_arr[$i] . "||";
						// 제일 앞에 빠지고 뒤에 새로운 트래픽 붙인다.
					} else {
						$realClass_session_e .= $real_val_arr[$i] . "||";
						// 제일 앞에 빠지고 뒤에 새로운 트래픽 붙인다.
					}
				}

				$traffic_str_arr = explode(",", $real_val_arr[$i]);

				for ($j = 0; $j < sizeof($traffic_str_arr); $j++) {
					$data_arr[$j] += $traffic_str_arr[$j];
				}
			}

			for ($k = 0; $k < sizeof($data_arr); $k++) {
				$json_data .= $json_cname_arr[$k] . ":" . round($data_arr[$k] / 10) . ",";
				// 평균수치를 jq Chart에 보낸다.
			}

			if ($dir == "i") {
				$realClass_session_i .= $traffic_str_real;
			} else {
				$realClass_session_e .= $traffic_str_real;
			}
		}
	}

	echo $traffic_unixtime . "," . substr($json_data, 0, -1) . "||" . $traffic_realtime . "," . $traffic_str;
	// json Echo

}

$socket_result = SocketConn($ip);

SocketSelect(array($sock), NULL, NULL);
$garbage_len = socket_recv($sock, $garbage_msg, 119, MSG_DONTWAIT);
if ($garbage_len != 119) {
	socket_close($sock);
	exit ;
}

$length_val = str_pad(strlen($class_cid_arr), 10, "0", STR_PAD_LEFT);
$msg = "TQP|0030|" . $length_val . "|" . $class_cid_arr;

//echo $msg ;

//do {
SocketSelect(NULL, array($sock), NULL);
socket_write($sock, $msg, strlen($msg));
// 실제로 소켓으로 보내는 명령어 //

SocketSelect(array($sock), NULL, NULL);
$len = socket_recv($sock, $prev_msg, 20, MSG_DONTWAIT);

if ($len != 20) {
	socket_close($sock);
	exit ;
}
$prev_msg_arr = explode("|", $prev_msg);

$tot_msg_len = 0;
$tot_real_msg = "";

while ($tot_msg_len != $prev_msg_arr[2]) {
	SocketSelect(array($sock), NULL, NULL);
	$msg_len = socket_recv($sock, $real_msg, $prev_msg_arr[2] - $tot_msg_len, MSG_DONTWAIT);
	$tot_msg_len += $msg_len;
	$tot_real_msg .= $real_msg;
}
// 소켓통신 통해 DATA 받아온다.

socket_close($sock);

$traffic_data = $tot_real_msg;
// cid별 Traffic 받아오기

$traffic_data_arr = explode(",", trim($traffic_data));
$traffic_unixtime = $traffic_data_arr[0];

/*
if ($timeVal_session == $traffic_unixtime . "" . $socket_conn_time) {
$f_open = fopen("/var/tmp/duplicate_unixtime", "a");
fwrite($f_open, $timeVal_session . "||" . $traffic_unixtime . "\n");
fclose($f_open);

$timeVal_session = $traffic_unixtime;
usleep(100000);
}
*/

//} while ( $timeVal_session == $traffic_unixtime ."". $socket_conn_time) ;

$timeVal_session = $traffic_unixtime . "" . $socket_conn_time;
$traffic_realtime = date("Y-m-d H:i:s", $traffic_unixtime);

for ($i = 1; $i < sizeof($traffic_data_arr); $i++) {
	$traffic_val_arr = explode(":", $traffic_data_arr[$i]);
	$traffic_cid = $traffic_val_arr[0];

	if ( $traffic_cid == 0 ) { // 2022-01-21 : 분석 클래스 선택 없는경우 cid '0' 값으로 2개 넘어온다. 하나만 처리 하도록 보완
		if ( $class_cnt > 0 ) break ; 
		$class_cnt ++ ;
	}
	//echo $traffic_cid . " || " ;

	//  if ( !$port_flag && $port_su > 1) {// ALL 선택한 경우
	if ( !$port_flag ) {// ALL 선택한 경우
		for ($j = 1; $j <= 64; $j = $j + 4) {
			//echo $j ."<br>";
			$k = ($j + 3) / 4;
			if ($k % 2 == 1) {// 외부방향
				$ext_traffic_val[$traffic_cid] += $traffic_val_arr[$j + 1] * 8;
				$ext_packet_val[$traffic_cid] += $traffic_val_arr[$j + 3] * 1;
			} else {// 내부방향
				//echo $traffic_val_arr[$j+1] . "<br>" ;
				$int_traffic_val[$traffic_cid] += $traffic_val_arr[$j + 1] * 8;
				$int_packet_val[$traffic_cid] += $traffic_val_arr[$j + 3] * 1;
			}
		}

		/// 2016-12-20 Timezone 연산후 YYYY-MM-DD HH:mm:ss 형식으로 반환
		$timezone_val = trim(read_conf("/usr/lib/qosd/conf/timezone.conf", "timezone")) ;
		date_default_timezone_set($timezone_val) ;
		$userTimezone = new DateTimeZone($timezone_val);
		$traffic_realtime = date("Y-m-d H:i:s", $traffic_unixtime);
		$traffic_realtime = '"' . $traffic_realtime . '"';

		$json_i_data .= '{"data":[' . $traffic_realtime . ',' . $int_traffic_val[$traffic_cid] . '],"label":"' . $cname_i_arr[$traffic_cid] . '","color":"#' . $col_arr[$i] . '"},';

		$json_e_data .= '{"data":[' . $traffic_realtime . ',' . $ext_traffic_val[$traffic_cid] . '],"label":"' . $cname_e_arr[$traffic_cid] . '","color":"#' . $col_arr[$i] . '"},';

		$json_i_data_pps .= '{"data":[' . $traffic_realtime . ',' . $int_packet_val[$traffic_cid] . '],"label":"' . $cname_i_arr[$traffic_cid] . '","color":"#' . $col_arr[$i] . '"},';

		$json_e_data_pps .= '{"data":[' . $traffic_realtime . ',' . $ext_packet_val[$traffic_cid] . '],"label":"' . $cname_e_arr[$traffic_cid] . '","color":"#' . $col_arr[$i] . '"},';

	} else {// 각 포트 선택한 경우

		//echo $port_flag . "=>" ;
		if (!$port_flag) $port_flag = "1";

		// 2016-04-26 : client 장비(2개 이상 장비인 경우) 수치 가져오는 위치 변경...(우선 장비인 경우만 처리)
		if ( is_file("/usr/lib/qosd/conf/client_set.conf") ) {
			if ( $device >= 2 && $port_flag > 2 ) {
				$j = ( $port_flag - 2 ) * 8;
				$e_t = $j - 6;
				$e_p = $j - 4;
				$i_t = $j - 2;
			} else if ( $device < 2 && $port_flag <= 2 ) {
				$j = $port_flag * 8;
				$e_t = $j - 6;
				$e_p = $j - 4;
				$i_t = $j - 2;
			} else {
				$j = -1 ;
				$e_t = -1; // 0으로 채우기 위해...
				$e_p = -1;
				$i_t = -1;
			}
		} else {
			$j = $port_flag * 8;
			$e_t = $j - 6;
			$e_p = $j - 4;
			$i_t = $j - 2;
		}

		$ext_traffic_val[$traffic_cid] = $traffic_val_arr[$e_t] * 8;
		$ext_packet_val[$traffic_cid] = $traffic_val_arr[$e_p] * 1;
		$int_traffic_val[$traffic_cid] = $traffic_val_arr[$i_t] * 8;
		$int_packet_val[$traffic_cid] = $traffic_val_arr[$j] * 1;

		/// 2016-12-20 Timezone 연산후 YYYY-MM-DD HH:mm:ss 형식으로 반환
		$timezone_val = trim(read_conf("/usr/lib/qosd/conf/timezone.conf", "timezone")) ;
		date_default_timezone_set($timezone_val) ;
		$userTimezone = new DateTimeZone($timezone_val);
		$traffic_realtime = date("Y-m-d H:i:s", $traffic_unixtime);
		$traffic_realtime = '"' . $traffic_realtime . '"';

		$json_i_data .= '{"data":[' . $traffic_realtime . ',' . $int_traffic_val[$traffic_cid] . '],"label":"' . $cname_i_arr[$traffic_cid] . '","color":"#' . $col_arr[$i] . '"},';

		$json_e_data .= '{"data":[' . $traffic_realtime . ',' . $ext_traffic_val[$traffic_cid] . '],"label":"' . $cname_e_arr[$traffic_cid] . '","color":"#' . $col_arr[$i] . '"},';

		$json_i_data_pps .= '{"data":[' . $traffic_realtime . ',' . $int_packet_val[$traffic_cid] . '],"label":"' . $cname_i_arr[$traffic_cid] . '","color":"#' . $col_arr[$i] . '"},';

		$json_e_data_pps .= '{"data":[' . $traffic_realtime . ',' . $ext_packet_val[$traffic_cid] . '],"label":"' . $cname_e_arr[$traffic_cid] . '","color":"#' . $col_arr[$i] . '"},';

	}
}

//if ($average == "true" && $sessCnt < 10) $sessCnt++;

$trafficInfo .= '"realTrafficIn":[' . substr($json_i_data, 0, -1) . '],';
$trafficInfo .= '"realTrafficOut":[' . substr($json_e_data, 0, -1) . '],';
$trafficInfo .= '"realTrafficInPPS":[' . substr($json_i_data_pps, 0, -1) . '],';
$trafficInfo .= '"realTrafficOutPPS":[' . substr($json_e_data_pps, 0, -1) . '],';

//return_func($direction, $int_traffic_val, $ext_traffic_val);
//socket_close($sock);

echo "{" . substr($trafficInfo, 0, -1) . "}";
?>
