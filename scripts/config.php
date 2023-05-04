<?php
#ini_set("display_errors", 0);
/* --------------------------------------------------------------------------------
 * config.php --
 *
 *  특정 변수값을 선언
 *
 * ------------------------------------------------------------------------------ */
$arr = $_POST ? $_REQUEST : $_REQUEST ;
foreach($arr as $key => $value) {
	    $$key = $value;
}

// COOKIE 처리
if (isset($_COOKIE['adminidCookie'])) {
	    foreach ($_COOKIE['adminidCookie'] as $name => $value) {
			$name = htmlspecialchars($name);
			$value = htmlspecialchars($value);
			 //echo "$name : $value <br />\n";

			$adminidCookie[$name] = $value ;
		}
	    $lan_sel = $_COOKIE['lan_sel'] ;
	    $lan_sel_css = $_COOKIE['lan_sel_css'] ;
}

// 파일 업로드 처리
$filename_name = $_FILES['filename']['name'] ;
$filename_size = $_FILES['filename']['size'] ;
$filename = $_FILES['filename']['tmp_name'] ;

// php_self 처리
$PHP_SELF = $_SERVER['PHP_SELF'] ;

global $config_php_included;

if ($config_php_included == 0)
{
    $config_php_included = 1;

    //*************************** 선택 language 설정 ******************************//
    if ($lan_sel == "")
    {
        $lan_sel = "kor";
    }
    //*************************** 선택 language 설정 ******************************//

    //****************************** MySQL 설정 ***********************************//
	if (is_file("/usr/lib/qosd/conf/localhost_set.conf")) {
		$f_open = fopen("/usr/lib/qosd/conf/localhost_set.conf", "r") ;
		$localhost_val = trim(fgets($f_open, 100)) ;
		fclose($f_open) ;

	} else {
		$localhost_val = "localhost" ;

	}
	if ($localhost_val != "localhost"  && !ereg("/", $PHP_SELF) ) {
		echo "<script>location.href='/manage_website.php';</script>" ;
	}
	//echo $localhost_val ; 

    $user["mysql_host"]               = $localhost_val ;
    $user["mysql_user"]               = "root";
	$user["mysql_user_hdd"]         = "root1";
    $user["mysql_pass"]               = "coffee";
    $user["mysql_db_mysql"]           = "mysql";
    $user["mysql_db_taskqos"]         = "qosconfig";
    $user["mysql_db_taskqos_hdd"]     = "qosconfig_hdd";
    //****************************** MySQL 설정 ***********************************//

    $is_demo_version = 0;                   // 데모버전인지, default=0(데모버전 아님)
    $is_detail_monitoring_view_menu = 0;    // 상세모니터링 메뉴를 보여줄것인지, default=0(안보임)
    $is_system_check_view_menu = 0;         // 시스템점검 메뉴를 보여줄것인지, default=0(안보임)
    $is_setting_change = 0;                 // 설정 변경이 있었는지, default=0(변경된 설정 없음)

    //********************************* 단위 ****************************************//
    define("BPS", 1);
    define("BYTE", 2);
    define("UNIT_KILO", 1000);
    define("UNIT_MEGA", 1000 * 1000);

    // $unit_array : bps단위, bps배수, bps 소수점, byte단위, byte배수, byte 소수점의 배열
    $unit_array =
        array(
            'bps_bits' => array('bps_view' => 'bps', 'bps_divide' => 1, 'bps_decimal' => 0,
                               'byte_view' => 'bps', 'byte_divide' => 1 / 8, 'byte_decimal' => 0),

            'bps_bytes' => array('bps_view' => 'bps', 'bps_divide' => 1, 'bps_decimal' => 0,
                                 'byte_view' => 'bytes', 'byte_divide' => 1, 'byte_decimal' => 0),

            'Kbps_Kbytes' => array('bps_view' => 'Kbps', 'bps_divide' => UNIT_KILO, 'bps_decimal' => 0,
                                   'byte_view' => 'Kbytes', 'byte_divide' => UNIT_KILO, 'byte_decimal' => 0),

            'Mbps_Mbytes_1' => array('bps_view' => 'Mbps', 'bps_divide' => UNIT_MEGA, 'bps_decimal' => 1,
                                     'byte_view' => 'Mbytes',  'byte_divide' => UNIT_MEGA, 'byte_decimal' => 0),

            'Mbps_Mbytes_3' => array('bps_view' => 'Mbps', 'bps_divide' => UNIT_MEGA, 'bps_decimal' => 3,
                                     'byte_view' => 'Mbytes',  'byte_divide' => UNIT_MEGA, 'byte_decimal' => 0),
        );

    $unit_normal_page = $unit_array["bps_bytes"];      // 페이지 전체적으로 공통적으로 사용되는 단위

    /********************************************************************************
        $unit_except_page : 공통 단위를 사용하지 않는 예외 페이지 array
            ! key : 파일이름
            ! vlaues : 단위배열 ($unit_array)

        <파일 이름>
        - now_graph                     // 실시간 그래프
        - now_graph_max                 // 실시간 그래프에서 최대값 설정
        - now_graph_packet              // 실시간 그래프에서 packet 현황보기
        - day_traffic                   // 당일 트래픽
        - class_traffic                 // 클래스별 트래픽
        - class_statistics              // 클래스별 통계
        - cut_class                     // 차단 클래스
        - all_network_analysis          // 전체네트워크분석
        - port_analysis                 // Port별 분석
        - ip_analysis                   // IP별 분석
        - protocol_analysis             // Protocol별 분석
    *********************************************************************************/
    $unit_except_page =
        array(
            'now_graph_max' => $unit_array["Kbps_Kbytes"],
            'now_graph_packet' => $unit_array["bps_bits"],
        );

	//********************************* 단위 ****************************************//

    //****************************** 디렉토리 및 파일이름 ****************************//
    $config_file_save["policy_save_and_recover"] = "qos_backup_local";
    $config_file_download["policy_save_and_recover"] = "qos_backup_remote";
    $config_file_ext["policy_save_and_recover"] = "bak";
    $config_file_limit_size["policy_save_and_recover"] = "100";
    $config_dir["policy_save_and_recover"] = "/var/qosd/conf_backup/";
    //****************************** 디렉토리 및 파일이름 ****************************//

    //**************** 사이트 차단의 분류등록 ****************//
    $site_kind_name_ar = array("$SHUT_MESSAGE[MSG_HARMFUL]","$SHUT_MESSAGE[MSG_SEXUAL]","$SHUT_MESSAGE[MSG_P2P]","$SHUT_MESSAGE[MSG_SEARCH]","$SHUT_MESSAGE[MSG_BLOG]","$SHUT_MESSAGE[MSG_MESS]","$PORT_KIND_MSG[255]");
    $site_kind_id_ar = array("1","2","3","4","5","6","9");
    //**************** 사이트 차단의 분류등록 ****************//

	//**************** Host관리 설정에서 Logging Item ****************//
    $host_logging_item_ar = array('1' => $HOST_MANAGE[15], '2' => $HOST_MANAGE[11], '3' => $HOST_MANAGE[16], '4' => $HOST_MANAGE[10],
											 '5' => $HOST_MANAGE[17], '6' =>$HOST_MANAGE[12], '7' =>$HOST_MANAGE[18], '8' =>$HOST_MANAGE[13],
											 '9' =>$HOST_MANAGE[19], '10' =>$HOST_MANAGE[20], '11' =>$HOST_MANAGE[21], '12' =>$HOST_MANAGE[22],
											 '13' =>$HOST_MANAGE[23], '14' =>$HOST_MANAGE[24]);
    //$host_logging_item_chk_ar = array("1","2","3","4","5","6","7","8","9","10","11","12","13","14");
	//**************** Host관리 설정에서 Logging Item ****************//

    //**************** 사이트 차단의 분류등록 ****************//

	//**************** 라이브러리 incldue ****************//
    include ("lib.php") ;
    //**************** 라이브러리 incldue ****************//

	$timezone_val = trim(read_conf("/usr/lib/qosd/conf/timezone.conf", "timezone")) ;

	if ( is_file("/etc/np4_m_dev") || is_file("/etc/TaskQosV_m_dev") ) {
		$dev_gubun = "1" ; 
	} else {
		$dev_gubun = "2" ;
	}

    date_default_timezone_set($timezone_val) ;
}
?>
