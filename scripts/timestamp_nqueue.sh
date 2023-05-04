#! /usr/lib/apm/php/bin/php -f

<?php
$cmd = "/usr/lib/qosd/www/html";
//**************** DB 연결 - 접근권한 체크 ***************//
require_once "config.php" ;
require_once "function_qos.php";
require_once 'deprecated.php';

$dbconn = mysql_connect($user['mysql_host'], $user['mysql_user'], $user['mysql_pass']);
$status = mysql_select_db($user['mysql_db_taskqos'], $dbconn);
//**************** DB 연결 - 접근권한 체크 ***************//

$filepath = "/usr/lib/qosd/tmp";
$filename = "timestamp_nqueue.info";

$nqueue_result = "[SERVICE_MODULE RCAL]\n";

if ($argc == 2) {

			//$hh = date('H') . "00" ;
			$hh = date('H', $argv[1]) . "" . date('i', $argv[1]) ;
			$week_val = date("w", $argv[1]) ;
			$week_day_eng = array('SUN', 'MON', 'TUE', 'WED', 'THU', 'FRI', 'SAT', 'SUN');
			$weekName = $week_day_eng[$week_val] ;	
			$now_select_qname = "" ;

			$query  = "select qname_id from tblSchedule where week = '$weekName' and from_time <= $hh and to_time > $hh " ;
			echo $query ; 
			$result = mysql_query($query, $dbconn);

			while ($row = mysql_fetch_row($result))
			{
				$now_select_qname_id = $row[0] ;
				$now_direction = $row[1] ;
				break ;
			}
			mysql_free_result($result) ;

			if (!strcmp($now_select_qname_id, ""))
			{
				$now_select_qname_id = 1 ;
			        $now_direction = "1" ;
			}

			$policy_result = "qid=".$now_select_qname_id . "\n";
			
			//기본 클래스 내부 외부 보여주기 위햇... 
			$sql = "select defcls from tblqdisc_int where qid=" . $now_select_qname_id ;
			$result = mysql_query($sql,$dbconn);
            $row = mysql_fetch_row($result);
			$default_name = $row[0];
            
			$sql = "select cid from tblClass_int where hid_qid=" . $now_select_qname_id . " and cname='$default_name' " ;
			$result = mysql_query($sql,$dbconn);
            $row = mysql_fetch_row($result);
			$default_int_cid = $row[0];

			$sql = "select cid from tblClass_ext where hid_qid=" . $now_select_qname_id . " and cname='$default_name' " ;
			$result = mysql_query($sql,$dbconn);
            $row = mysql_fetch_row($result);
			$default_ext_cid = $row[0];

            $default_cid = "default=".$default_int_cid . "," . $default_ext_cid . "\n";
			//기본 클래스 내부 외부 보여주기 위햇...

			// 정책에서 bandwidth  읽기..
			$sql = "select int_band, ext_band, use_both_dir, qname, port_chk, qtype_1 from tblqname where qname_id = $now_select_qname_id ";
			$result = mysql_query($sql,$dbconn);
       		$row = mysql_fetch_row($result);     
			//while ( $row = mysql_fetch_row($result)) {
			$tot_rate = $row[0];
			$now_direction = $row[2];
			$port_chk = $row[4] ;
			$qtype_1 = $row[5] ;

			if ($port_chk == "1") {
				$qos_type = "port" ; 
			} else {
				$qos_type = "all" ;
			}
			
			// 2010-05-25 : 클래스로그 설정에 따라 확인
			if ($qtype_1 == "1" || $qtype_1 == "3") {
				$qtype_val = "class_log=yes" ;
			} else {
				$qtype_val = "class_log=no" ;
			}

			$rate_result = "tot_rate=".$row[0].",".$row[1]."\n"; 
			$policy_result .= "qname=" . $row[3] . "\n";
			$policy_result .= "qos_type=" . $qos_type . "\n";
			$policy_result .= $qtype_val . "\n";
			//}
			// 정책에서 bandwidth  읽기..

           	if ($now_direction == "1") { // 양방향
					// 클래스에서 leaf  읽기..
					//$sql = "select count(cid) from tblClass_int where leaf=1 and hid_qid = $now_select_qname_id "; 
					$sql = "select hid_cid, rate from tblClass_int where leaf=1 and hid_qid = $now_select_qname_id "; // 2019-05-13 : 보정값 total 표시 위해 처리
					$result = mysql_query($sql,$dbconn);
					$leaf_su = mysql_num_rows($result) ; 

					while ( $row = mysql_fetch_row($result)) { // 2019-05-13 : 보정값 total 표시 위해 처리
					   $rate_tot[$row[0]] += $row[1] ; 
					   $rate_tot_val += $row[1] ;
					   //$leaf_su = $row[0];
					}
					// 클래스에서 leaf  읽기..

					$rate_tot_result = "rate_total=B," . $rate_tot_val . "," . $tot_rate . "\n" ; // 2019-05-13 : 보정값 total 표시 위해 처리

					// 클래스에서 자신이 부모인경우  읽기..
					$sql = "select count(cid) from tblClass_int where cid = hid_cid and hid_qid = $now_select_qname_id ";
					$result = mysql_query($sql,$dbconn);

					while ( $row = mysql_fetch_row($result)) {
					   $parent_su = $row[0];
					}
					// 클래스에서 leaf  읽기..

					$num_nqueue_result = "num_nqueue=".$leaf_su . "," . $parent_su . "\n";

					// 클래스전체파일 읽기..
					$sql = "select cid, rate, ceil, leaf, hid_cid, cname, is_view, prio from tblClass_int where hid_qid = $now_select_qname_id order by hid_cid asc, cid asc";
					$result = mysql_query($sql,$dbconn);

					while ( $row = mysql_fetch_row($result)) {
					   //$rate_rate = $row[1] / $tot_rate * 100 ;  
					   //$ceil_rate = $row[2] / $tot_rate * 100 ;  
					   $rate_rate = $row[1] ;  
					   if ( $rate_tot[$row[0]] > 0 ) $rate_rate = $rate_tot[$row[0]] ; // 2019-05-13 : 보정값 total 표시 위해 처리

					   $ceil_rate = $row[2] ; 
					   $cname = $row[5] ; 
					   $is_view = $row[6] ;
					   $prio = $row[7] ;
					   $class_result .= "nqueue=B," . $row[0] .",". $row[4] .",". $row[3] .",". $rate_rate .",". $ceil_rate .",". $cname .",". $is_view .",". $prio . "\n" ;  
					}

			} else { // 각방향
				// 클래스에서 leaf  읽기..
					//$sql = "select count(cid) from tblClass_int where leaf=1 and hid_qid = $now_select_qname_id ";
					$sql = "select hid_cid, rate from tblClass_int where leaf=1 and hid_qid = $now_select_qname_id "; // 2019-05-13 : 보정값 total 표시 위해 처리
					$result = mysql_query($sql,$dbconn);
					$leaf_su = mysql_num_rows($result) ; 

					while ( $row = mysql_fetch_row($result)) { // 2019-05-13 : 보정값 total 표시 위해 처리
					   $rate_tot[$row[0]] += $row[1] ; 
					   $i_rate_tot_val += $row[1] ;
					   //$leaf_su = $row[0];
					}
					// 클래스에서 leaf  읽기..
					
					$rate_tot_result = "rate_total=I," . $i_rate_tot_val . "," . $tot_rate . "\n" ; // 2019-05-13 : 보정값 total 표시 위해 처리

					// 클래스에서 자신이 부모인경우  읽기..
					$sql = "select count(cid) from tblClass_int where cid = hid_cid and hid_qid = $now_select_qname_id ";
					$result = mysql_query($sql,$dbconn);

					while ( $row = mysql_fetch_row($result)) {
					   $parent_su = $row[0];
					}
					// 클래스에서 leaf  읽기..

					$int_leaf_su = $leaf_su ;
					$int_parent_su = $parent_su ;

					// 클래스전체파일 읽기..
					$sql = "select cid, rate, ceil, leaf, hid_cid, cname, is_view, prio  from tblClass_int where hid_qid = $now_select_qname_id order by hid_cid asc, cid asc";
					$result = mysql_query($sql,$dbconn);

					while ( $row = mysql_fetch_row($result)) {
					   //$rate_rate = $row[1] / $tot_rate * 100 ;  
					   //$ceil_rate = $row[2] / $tot_rate * 100 ;  
					   $rate_rate = $row[1] ;  
					   if ( $rate_tot[$row[0]] > 0 ) $rate_rate = $rate_tot[$row[0]] ; // 2019-05-13 : 보정값 total 표시 위해 처리

					   $ceil_rate = $row[2] ;  
					   $cname = $row[5] ;
                       $is_view = $row[6] ;	
					   $prio = $row[7] ;
					   $class_result .= "nqueue=I," . $row[0] .",". $row[4] .",". $row[3] ."," . $rate_rate ."," . $ceil_rate .",". $cname .",". $is_view .",". $prio ."\n" ;  
					}

					// 클래스에서 leaf  읽기..
					//$sql = "select count(cid) from tblClass_ext where leaf=1 and hid_qid = $now_select_qname_id ";
					$sql = "select hid_cid, rate from tblClass_ext where leaf=1 and hid_qid = $now_select_qname_id "; // 2019-05-13 : 보정값 total 표시 위해 처리
					$result = mysql_query($sql,$dbconn);
					$leaf_su = mysql_num_rows($result) ; 

					while ( $row = mysql_fetch_row($result)) { // 2019-05-13 : 보정값 total 표시 위해 처리
					   $rate_tot[$row[0]] += $row[1] ; 
					   $e_rate_tot_val += $row[1] ;
					   //$leaf_su = $row[0];
					}
					// 클래스에서 leaf  읽기..
					
					$rate_tot_result .= "rate_total=E," . $e_rate_tot_val . "," . $tot_rate . "\n" ; // 2019-05-13 : 보정값 total 표시 위해 처리

					// 클래스에서 자신이 부모인경우  읽기..
					$sql = "select count(cid) from tblClass_ext where cid = hid_cid and hid_qid = $now_select_qname_id ";
					$result = mysql_query($sql,$dbconn);

					while ( $row = mysql_fetch_row($result)) {
					   $parent_su = $row[0];
					}
					// 클래스에서 leaf  읽기..

					$leaf_su = $int_leaf_su + $leaf_su ;
					$parent_su = $int_parent_su + $parent_su ;

					// 클래스전체파일 읽기..
					$sql = "select cid, rate, ceil, leaf, hid_cid, cname, is_view, prio from tblClass_ext where hid_qid = $now_select_qname_id order by hid_cid asc, cid asc";
					$result = mysql_query($sql,$dbconn);

					while ( $row = mysql_fetch_row($result)) {
					   //$rate_rate = $row[1] / $tot_rate * 100 ;  
					   //$ceil_rate = $row[2] / $tot_rate * 100 ;  
					   $rate_rate = $row[1] ;  
					   if ( $rate_tot[$row[0]] > 0 ) $rate_rate = $rate_tot[$row[0]] ; // 2019-05-13 : 보정값 total 표시 위해 처리

					   $ceil_rate = $row[2] ;  
					   $cname = $row[5] ;
                       $is_view = $row[6] ;	
					   $prio = $row[7] ;
					   $class_result .= "nqueue=E," . $row[0] .",". $row[4] .",". $row[3] ."," . $rate_rate ."," . $ceil_rate .",". $cname .",". $is_view .",". $prio ."\n" ;  
					}
					
					$num_nqueue_result = "num_nqueue=".$leaf_su . "," . $parent_su . "\n";
			}	

			//$f_open = fopen("/usr/lib/qosd/conf/nqueue.conf","r");
			//$nqueue_val = trim(fgets($f_open,1024)) . "\n";
			//fclose($f_open);

			$nqueue_result .= "start=off\n" . $policy_result ."". $default_cid ."". $num_nqueue_result ."". $rate_result ."". $rate_tot_result . "" . $class_result ."". $nqueue_val ."start=on\n" ;
			//echo $nqueue_result;

} else {
	$nqueue_result .= "Usage $argv[0] [start_time] [save filename]";
}

$f_open = fopen("$filepath/$filename","w");
fwrite($f_open,$nqueue_result);
fclose($f_open);

//exec("cat $filepath/$filename > /proc/service/setting");
//echo $result_cmd;
// 입력파일 읽고 쓰기.
?>

