<?
/* --------------------------------------------------------------------------------
 * function_qos.php --
 *
 * IPv4 주소변환 함수
 *
 * ------------------------------------------------------------------------------ */
function sizeCheckPercent_P($va1, $va2, $va3)
{
    $rePercent = "" ;

    if (($va1 < 10)
            && ($va1 > 0))
    {
        $rePercent = $va2 . "." . substr($va3, 0, 1) ;
    }
    else if ($va1 == 0)
    {
        $rePercent = "0.1" ;
    }
    else
    {
        $rePercent = $va1 ;
    }

    return $rePercent ;
}

function img_check($val)
{
	//if ( $val > 60 ) $val = $val - 60 ; 
    return sprintf("img_graph%02d.gif", $val);
}

function WeekDayEng($val)
{
    $week_day_eng = array('SUN', 'MON', 'TUE', 'WED', 'THU', 'FRI', 'SAT', 'SUN');

    return $week_day_eng[$val] ;
}

function WeekDay($val)
{
    $week_day_kor = array('&nbsp;', '월', '화', '수', '목', '금', '토', '일');

    return $week_day_kor[$val] ;
}

//******************* MIN VALUE GET ***********************//
function check_min($ip_c, $p_nShiftValue)
{
    if ($ip_c < 0)
    {
        $ip_c = 0;
    }
    else if ($ip_c > 255)
    {
        $ip_c = 255;
    }

    $nDivideValue = $ip_c >> $p_nShiftValue;
    $ip_c = ($nDivideValue << $p_nShiftValue);

    return $ip_c;
}

function mod_min_check($ip_c,$va)
{
    if ($va < 0
            || $va > 8)
    {
        $ret = 0;
    }
    else
    {
        $ret = check_min($ip_c, 8 - $va);
    }

    return $ret ;
}

function min_get($ip_v, $mask_v)
{
    $split_ip = explode(".", $ip_v) ;
    $mask_div = $mask_v >> 3;
    $mask_mod = $mask_v & 7;

    if ($mask_div == 0)
    {
        $ip_check = $split_ip[0] ;
        $ip = mod_min_check($ip_check, $mask_mod) . ".x.x.x" ;
        $ip = mod_min_check($ip_check, $mask_mod) . "." ;
    }
    else if ($mask_div == 1)
    {
        $ip_check = $split_ip[1] ;
        $ip = $split_ip[0] . "." . mod_min_check($ip_check, $mask_mod) . ".x.x" ;
        $ip = $split_ip[0] . "." . mod_min_check($ip_check, $mask_mod) . "." ;
    }
    else if ($mask_div == 2)
    {
        $ip_check = $split_ip[2] ;
        $ip = $split_ip[0] . "." . $split_ip[1] . "." . mod_min_check($ip_check, $mask_mod) . ".x" ;
        $ip = $split_ip[0] . "." . $split_ip[1] . "." . mod_min_check($ip_check, $mask_mod) . "." ;
    }
    else if ($mask_div == 3)
    {
        $ip_check = $split_ip[3] ;
        $ip = $split_ip[0] . "." . $split_ip[1] . "." . $split_ip[2] . "." . mod_min_check($ip_check, $mask_mod) ;
    }
    else if ($mask_div == 4)
    {
        $ip_check = $ip_v ;
        $ip = $ip_check ;
    }
    else
    {
        $ip = "0.0.0.0" ;
    }

    return $ip ;
}
//******************* MIN VALUE GET ***********************//

//******************* MAX VALUE GET ***********************//
function check_max($ip_c, $p_nShiftValue)
{
    if ($ip_c < 0)
    {
        $ip_c = 0;
    }
    else if ($ip_c > 255)
    {
        $ip_c = 255;
    }

    $nDivideValue = $ip_c >> $p_nShiftValue;
    $ip_c = (($nDivideValue + 1) << $p_nShiftValue) - 1;

    return $ip_c;
}

function mod_max_check($ip_c,$va)
{
    if ($va < 0
            || $va > 8)
    {
        $ret = 0;
    }
    else
    {
        $ret = check_max($ip_c, 8 - $va);
    }

    return $ret ;
}

function max_get($ip_v, $mask_v)
{
    $split_ip = explode(".", $ip_v) ;
    $mask_div = $mask_v / 8 ;
    $mask_mod = $mask_v % 8 ;

    if ($mask_div == 0)
    {
        $ip_check = $split_ip[0] ;
        $ip = mod_max_check($ip_check, $mask_mod) . ".x.x.x" ;
        $ip = mod_max_check($ip_check, $mask_mod) . "." ;
    }
    else if ($mask_div == 1)
    {
        $ip_check = $split_ip[1] ;
        $ip = $split_ip[0] . "." . mod_max_check($ip_check, $mask_mod) . ".x.x" ;
        $ip = $split_ip[0] . "." . mod_max_check($ip_check, $mask_mod) . "." ;
    }
    else if ($mask_div == 2)
    {
        $ip_check = $split_ip[2] ;
        $ip = $split_ip[0] . "." . $split_ip[1] . "." . mod_max_check($ip_check, $mask_mod) . ".x" ;
        $ip = $split_ip[0] . "." . $split_ip[1] . "." . mod_max_check($ip_check, $mask_mod) . "." ;
    }
    else if ($mask_div == 3)
    {
        $ip_check = $split_ip[3] ;
        $ip = $split_ip[0] . "." . $split_ip[1] . "." . $split_ip[2] . "." . mod_max_check($ip_check, $mask_mod) ;
    }
    else if ($mask_div == 4)
    {
        $ip_check = $ip_v ;
        $ip = $ip_check ;
    }
    else
    {
        $ip = "0.0.0.0" ;
    }

    return $ip ;
}
//******************* MAX VALUE GET ***********************//
?>
