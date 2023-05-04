<?

/* --------------------------------------------------------------------------------
 * lib.php --
 *
 * ���̺귯�� �Լ�
 *
 * ------------------------------------------------------------------------------ */

function alert($msg)
{
    echo "<script>\nalert(\"$msg\");\n</script>";
}

function history_back($msg = "")
{
    if ($msg != "")
    {
        echo "<script>\n alert(\"$msg\") \n history.go(-1);\n</script>";
    }
    else
    {
        echo "<script>\n history.go(-1);\n</script>";
    }

    exit;
}

function self_close($msg = "")
{
    if ($msg != "")
    {
        echo "<script>\n alert(\"$msg\") \n self.close();\n</script>";
    }
    exit;
}

function parent_history_back($msg = "")
{
    if ($msg != "")
    {
        echo "<script>\n alert(\"$msg\") \n parent.history.go(-1);\n</script>";
    }
    else
    {
        echo "<script>\n parent.history.go(-1);\n</script>";
    }

    exit;
}

function location_href($url, $msg = "")
{
    if ($msg != "")
    {
        echo "<script>\n alert(\"$msg\") \n location.href='$url';\n</script>";
    }
    else
    {
        echo "<script>\nlocation.href='$url';\n</script>";
    }

    exit;
}

function iframe_parent_reload($msg = "")
{
    if ($msg != "")
    {
        echo "<script>\n alert(\"$msg\") \n self.close();\n opener.parent.location.reload();\n</script>";
    }
    else
    {
        echo "<script>\n self.close();\n opener.parent.location.reload();\n</script>";
    }

    exit;
}

function iframe_reload($msg = "")
{
    if ($msg != "")
    {
        echo "<script>\n alert(\"$msg\") \n parent.location.reload();\n</script>";
    }
    else
    {
        echo "<script>\n parent.location.reload();\n</script>";
    }

    exit;
}

function parent_location_href($parent_url, $msg = "")
{
    if ($msg != "")
    {
        echo "<script>\n alert(\"$msg\") \n parent.location.href='$parent_url';\n</script>";
    }
    else
    {
        echo "<script>\n parent.location.href='$parent_url';\n</script>";
    }

    exit;
}

function opener_location_href($opener_url, $url = "", $msg = "")
{
    if ($msg != "")
    {
        if ($url != "")
        {
            echo "<script>\n try {\n ".
                 "opener.location.href = '$opener_url';\n } catch(e) {}\n ".
                 "\n alert(\"$msg\") \n location.href='$url';\n</script>";
        }
        else
        {
            echo "<script>\n try {\n ".
                 "opener.location.href = '$opener_url';\n } catch(e) {}\n ".
                 "\n alert(\"$msg\") \n window.close ();\n</script>";
        }
    }
    else
    {
        if ($url != "")
        {
            echo "<script>\n try {\n ".
                 "opener.location.href = '$opener_url';\n } catch(e) {}\n ".
                 "location.href='$url';\n</script>";
        }
        else
        {
            echo "<script>\n try {\n ".
                 "opener.location.href = '$opener_url';\n } catch(e) {}\n ".
                 "window.close ();\n</script>";
        }
    }

    exit;
}

function decode_function($p_sValue)
{
    $sReturn = "" ;
    $p_sValue = trim($p_sValue) ;

    for ($i = 0; $i < strlen($p_sValue); $i++)
    {
        $adminid_ar[] = substr($p_sValue, $i, 3) ;
        $i = $i + 2 ;
    }

    for ($i = 0 ; $i < sizeof($adminid_ar) - 1; $i++)
    {
        $sReturn = $sReturn . chr((int)($adminid_ar[$i] - $adminid_ar[$i+1])) ;
        $i = $i + 2 ;
    }

    return $sReturn ;
}

function hangle_check($String)
{
    $StringLen = strlen($String);  // ���� ���ڿ��� ���̸� ����
    $RetStr = substr($String, 0, 2);
    $RetStr2 = substr($String, 2, $StringLen);
    return $RetStr . $RetStr2 ;
}

function sub_string($string, $start, $length,$charset=NULL) {
        if($charset==NULL) {
                $charset='UTF-8';
        }
        /* ��Ȯ�� ���ڿ��� ���̸� ����ϱ� ����, mb_strlen �Լ��� �̿� */
        $str_len=mb_strlen($string, $charset);

		 $han = 0;
		 $eng = 0;
		 $title = strip_tags($string);
		 $temp = substr($title, 0, $length);

		 for($k=0;$k<strlen($temp);$k++)
		 {
				  if(ord($temp[$k]) > 127) {
						  $han++;
				  } else {
						  $eng++;
				  }
		 }
		 //echo $han . " || " . $eng . "<br>" ; 
		 //if ( $han % 2 ) { $han++; }
		 $han = $han / 2 ; 
		 $total_size = $eng +$han;
		 //echo $temp . " || " . $length . " || " . $total_size . "<br>" ; 
	
        if($str_len > $total_size) {
			/* mb_substr  PHP 4.0 �̻�, iconv_substr PHP 5.0 �̻� */
			$string=mb_substr($string,$start,$total_size, $charset);
			$string.="...";
        }

        return $string;

}

function Title_cut($title, $value) {
	//if (strlen($title) > $value)
	//{
			$title_real = sub_string($title, 0, $value, "UTF-8") ;
			/*
			 $han = 0;
			 $eng = 0;
			 $title =strip_tags($title);
			 $temp=substr($title,0,$value);

					 for($k=0;$k<strlen($temp);$k++)
					 {
							  if(ord($temp[$k]) > 127) {
									  $han++;
							  } else {
									  $eng++;
							  }
					 }

			 if($han%2){ $han++; }
			 $total_size = $eng+$han;
			 $title_real = substr($title,0,$total_size);
			 $title_real = $title_real."...";
			*/

			//$title_real = iconv_substr($title, 0, $value, "utf-8") . "..." ;

			return $title_real; // ���� �κ�(15�� �̻��� '...' ����)



	//} else {

	//       return $title;
	//}
}

function read_conf($url,$string){
    if (!file_exists($url)) {
	return false;
    }
    $f_open = fopen($url,"r");
	while(!feof($f_open))
	{
		$rr = fgets($f_open) ;
		$rr_array = explode("=",$rr) ;
        if ($rr_array[0] == $string){
           break;
		}
	}
    fclose($f_open);
	return $rr_array[1];
}

function modify_conf($url,$string,$val){
    $f_open = fopen($url,"r");
	while(!feof($f_open))
	{
		$rr = fgets($f_open) ;
		$rr_array = explode("=",$rr) ;
        if ($rr_array[0] == $string){
		    $arr_string .= $rr_array[0] . "=" . $val . "\n";           
		} else if ($rr_array[0]) {
            $arr_string .= $rr ;
		}
	}
    fclose($f_open);

	$f_open = fopen($url,"w");
	fwrite($f_open,$arr_string);
	fclose($f_open);
}

function ip_expand($org_ip) {
	if ($org_ip) {
        $ip_val = explode("/", $org_ip) ;
        $ip_arr = explode(".", $ip_val[0]) ;
        $ip = 0;
        $ip |= $ip_arr[0] << 24 ; // 192
        $ip |= $ip_arr[1] << 16 ; // 168
        $ip |= $ip_arr[2] << 8 ; // 100
        $ip |= $ip_arr[3]; // 0
        $mask_val = $ip_val[1]; // 0

        //echo $mask_val . "\n";
        $count =  0x7fffffff >> ( $mask_val - 1 ) ;

        //echo "count : " . $count . "\n";
        $mask = ~$count;
        $start_ip = $ip & $mask;

        for( $i=0 ; $i<=$count ; $i++ )
        {
	           $ip = 0;
               $ip = $start_ip + $i;
               $return_ip_val .= (($ip>>24)&0x000000ff) . "." . (($ip>>16)&0x000000ff) . "." . (($ip>>8)&0x000000ff) . "." . ($ip&0x000000ff) . ";" ;
        }

		return $return_ip_val ;

	}
}

function ipv6_conv_func($val) {
	$ip_arr = explode(":", $val) ;
	$j = 0 ; 
	$k = 0 ;
	for ($i=0 ; $i<sizeof($ip_arr) ; $i++) {
		if (!$ip_arr[$i]) {
			$j++ ; 

			if ( $k == 0) {
				if ($i+1 == sizeof($ip_arr)) {
					$re_val .= "::" ; 
				} else {
					continue;
				}
			} else {
				$re_val .= $ip_arr[$i] . ":" ; 
			}
			
		} else {
			
			if ( $j > 1 ) {
				if ( $k == 0 ) { 
					$re_val .= ":" ; 
					$re_val .= $ip_arr[$i] . ":" ; 
					$k++ ; 
				} else {
					$re_val .= $ip_arr[$i] . ":" ; 
				}
				$j = 0 ;

			} else if ( $j > 0 ) {
				$re_val .= $ip_arr[$i-1] . ":" . $ip_arr[$i] . ":" ; 
				$j = 0 ; 

			} else {
				$re_val .= $ip_arr[$i] . ":" ; 
			}

		}
	}
	
	return substr($re_val, 0, -1) ; 
}

// 2015-03-06 : csv ����� ���� ǥ��ó��
function bps_unit_func($val) {
	if ( $val == 0 ) {
		$unit_val = "bps" ; 
	} else if ( $val == 1 ) {
		$unit_val = "Kbps" ;
	} else if ( $val == 2 ) {
		$unit_val = "Mbps" ;
	} else if ( $val == 3 ) {
		$unit_val = "bytes" ;
	} else if ( $val == 4 ) {
		$unit_val = "Kbytes" ;
	} else if ( $val == 5 ) {
		$unit_val = "Mbytes" ;
	}

	return $unit_val ; 
}
// 2015-03-06 : csv ����� ���� ǥ��ó��

// 2010-05-26 : ���� ����
function BpsInterval_func($val, $unit_val, $diff) {
	if ($unit_val == "0") { 
		$traffic_val = $val * 8 / $diff ; 
	} else if ($unit_val == "1") {
		$traffic_val = ( $val * 8 / $diff ) / 1000;
	} else if ($unit_val == "2") {
		$traffic_val = ( $val * 8 / $diff ) / 1000 / 1000 ;
	} else if ($unit_val == "3") {
		$traffic_val = $val  ;
	} else if ($unit_val == "4") {
		$traffic_val = $val / 1024 ;
	} else if ($unit_val == "5") {
		$traffic_val = $val / 1024 / 1024 ;
	}
	return $traffic_val ;
}

function NumberFormat_func($val, $unit_val, $search_gubun) {
	if ( $unit_val != "0" && $unit_val != "3" && $search_gubun == "BYTE" ) { 
		$traffic_val = number_format($val, 2) ; 
	} else {
		$traffic_val = number_format($val) ; 
	}
	return $traffic_val ;
}
// 2010-05-26 : ���� ����

// 2015-03-10 : csv ����� ���� ǥ��ó��
function category_func($val, $dir_val) {
	
	
	global $CLASS_MONITOR_MSG, $MONITORING_MSG, $TRAFFIC_MANAGE, $PACKET_MSG, $TRAFFIC_DROP, $SYSTEM_MANAGE, $NETWORK_MSG ;

	if ( $val == 1 || $dir_val == "class" ) {
		$category_val = $CLASS_MONITOR_MSG[41]  ; // ��å�м�
		$val = "1" ; 

	} else if ( $val == 2  || $dir_val == "setting" ) {
		$category_val = $CLASS_MONITOR_MSG[42]  ; // ��å����
		$val = "2" ;

	} else if ( $val == 4  || $dir_val == "traffic_settings" ) {
		$category_val = $TRAFFIC_MANAGE[84] ; // Ʈ���� ����
		$val = "4" ;

	} else if ( $val == 3  || $dir_val == "monitoring" ) {
		$category_val = $MONITORING_MSG[66] ; // Ʈ���� �м�
		$val = "3" ;

	} else if ( $val == 5  || $dir_val == "packet_settings" ) {
		$category_val = $PACKET_MSG[26] ; // Ʈ���� ����
		$val = "5" ;

	} else if ( $val == 6  || $dir_val == "traffic_drop" ) {
		$category_val = $TRAFFIC_DROP[5] ; // Ʈ���� ����
		$val = "6" ;

	} else if ( $val == 9  || $dir_val == "system_setting" ) {
		$category_val = $SYSTEM_MANAGE[HELP_TITLE] ; // �ý��� ����
		$val = "9" ;

	} else if ( $val == 10  || $dir_val == "admin_setting" ) {
		$category_val = $NETWORK_MSG[1] ; // ������ ����
		$val = "10" ;

	}
	//return $category_val ; 
	return $val ; 

}
// 2015-03-06 : csv ����� ���� ǥ��ó��

// 2016-03-29 : ���� ��å ���̵�/���� �������� 
function now_qname_set_f() { 
	global $dbconn ; 

	$weekName = strtoupper(date("D")) ;
	$hh = date("H") . date("i") ;

	$q_query  = "select qname_id, target from tblSchedule where week = '$weekName' and from_time <= $hh and to_time > $hh " ;
	$q_result = mysql_query($q_query, $dbconn);
	$q_row = mysql_fetch_row($q_result);
	$qname_id = $q_row[0] ;
	
	$sql = "select qname_id, use_both_dir from tblqname where qname_id = '$qname_id' " ; 
	$result = mysql_query($sql, $dbconn) ; 
	$row = mysql_fetch_row($result) ; 
	
	$now_qname_set_arr[] = $row[0] ;
	$now_qname_set_arr[] = $row[1] ;
	
	return $now_qname_set_arr ; 
}
// 2016-03-29 : ���� ��å ���̵�/���� �������� 

function convert_encode_func($val, $to_encode) {
	if ( $to_encode ) { // 63bit
		if ( $to_encode == "UTF-8" ) {
			$val = mb_convert_encoding($val, $to_encode, "EUC-KR") ; 
		} else {
			$val = mb_convert_encoding($val, $to_encode, "UTF-8") ; 
		}
			
	} 
	
	return $val ; 

}

function UnitBpsByteFunc($val, $gubun) {
	if  ( $gubun == "bps" ) {
		if ($val >= 1000 * 1000 * 1000) {
			$unit_val = "Gbps" ;
			$traffic_val = number_format($val / (1000 * 1000 * 1000), 2) ;
		} else if ( $val >= 1000 * 1000 ) {
			$unit_val = "Mbps" ;
			$traffic_val = number_format($val / (1000 * 1000), 2) ;
		} else if ( $val >= 1000 ) {
			$unit_val = "Kbps" ;
			$traffic_val = number_format($val / 1000, 2) ;
		} else {
			$unit_val = "bps" ;
			$traffic_val = number_format($val, 2) ; 
		}

	} else if ( $gubun == "byte" ) {
		if ($val >= 1024 * 1024 * 1024) {
			$unit_val = "Gbyte" ;
			$traffic_val = number_format($val / (1024 * 1024 * 1024), 2) ;
		} else if ( $val >= 1024 * 1024 ) {
			$unit_val = "Mbyte" ;
			$traffic_val = number_format($val / (1024 * 1024), 2) ;
		} else if ( $val >= 1024 ) {
			$unit_val = "Kbyte" ;
			$traffic_val = number_format($val / 1024, 2) ;
		} else {
			$unit_val = "byte" ;
			$traffic_val = number_format($val, 2) ; 
		}

	}

	return $traffic_val . " " . $unit_val ;
}

// >>>>>>> 230419 hjkim - PHP-SQL Validator
// define("DEBUG", 1); 
function D($str) {	if(defined("DEBUG") == true) print_r($str."<br>");  }
$LINE_GAP = 10;
$DB_TEXT_FIELD_SIZE = 65535;
function php_sql_validator($file, $line, $q) {
	global $LINE_GAP, $DB_TEXT_FIELD_SIZE;
	D("{$file}:L{$line}");
	
	switch("{$file}:L{$line}") {
		case "port_group_add_apply.php:L113":
			// >>>>>>> 230418 hjkim - 310 Port �׷� ����
			// D("-- q : {$q}");
			$values_pos = strpos($q, "values(");
			$values_csv = substr($q, $values_pos+7, -1);
			// D("-- value_stmt : {$values_csv}");
			$csv_arr = explode(',', $values_csv);
			$view_port = $csv_arr[3]; // view_port �÷�=3��°
			$real_port = $csv_arr[4]; // real_port �÷�=4��°
			D("view_port:{$view_port}");
			// >>>>>>> 230418 hjkim - 13211 �߰��� ��Ʈ ���� ���� ���׼���
			if(strlen($real_port) > $DB_TEXT_FIELD_SIZE) { 
				history_back("Port range exceeded.(".strlen($real_port).")");
			}
			// <<<<<<< 230418 hjkim - 13211 �߰��� ��Ʈ ���� ���� ���׼���
			// <<<<<<< 230418 hjkim - 310 Port �׷� ����
		break;

		default:
			D("Has Not Validator.");
		break;
	}
}
// <<<<<<< 230419 hjkim - PHP-SQL Validator

function mysqliQuery($query, $conn) { 
	
	// >>>>>>> 230419 hjkim - PHP-SQL Validator
	$backtrace = debug_backtrace();
	$be_inc_file = basename($backtrace[0]["file"]);
	$be_inc_line = basename($backtrace[0]["line"]);

	if( count($backtrace) != 0) {
		D("be_included_file: {$be_inc_file}");
		D("be_included_line: {$be_inc_line}");
		php_sql_validator($be_inc_file, $be_inc_line, $query);
	}

	// <<<<<<< 230419 hjkim - PHP-SQL Validator
	$result =  mysql_query($query, $conn) ;
	return $result ; 
}


?>
