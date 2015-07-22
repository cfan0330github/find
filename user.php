<?php
/*********************/
/*                   */
/*  Dezend for PHP5  */
/*         NWS       */
/*      Nulled.WS    */
/*                   */
/*********************/

$_versionID = "\$iGENUS: igenus/admroot/user.php,v 1.6 2008/11/11 08:55:39 dengxuan Exp \$";
require_once( "../adminclude/showid.php" );
require_once( "../adminclude/include_common.php" );
require_once( "../include/class_user.php" );
require_once( "../include/class_ftpquota.php" );
if ( !$domain_privi->dopriv( SYSTEM_VISIT_MANAGE, "user_privi" ) && !$domain_privi->dopriv( SYSTEM_GLOBAL_MANAGE, "user_privi" ) )
{
    alertexit( $LANG_DOMAIN_USER_LIST_PRIVI );
}
$get_manager = $_GET['manager'];
$managerStr = "";
if ( $domain_privi->dopriv( SYSTEM_VISIT_MANAGE, "user_privi" ) && !$domain_privi->dopriv( SYSTEM_GLOBAL_MANAGE, "user_privi" ) )
{
    $managerStr = "AND manager_id='".$_SESSION['G_ADMID']."'";
}
else if ( $domain_privi->dopriv( SYSTEM_VISIT_MANAGE, "user_privi" ) && $domain_privi->dopriv( SYSTEM_GLOBAL_MANAGE, "user_privi" ) && $get_manager != "" )
{
    if ( $domain_privi->manager_arr[$get_manager] == "postmaster" )
    {
        $managerStr = "AND (manager_id='".$get_manager."' OR manager_id='0')";
    }
    else
    {
        $managerStr = "AND manager_id='".$get_manager."'";
    }
}
$_SESSION['G_REFERER_URL'] = $_SERVER['REQUEST_URI'];
unset( $_SESSION['USER_INFO'] );
$get_page = $_GET['page'];
$get_keyword = $_GET['keyword'];
$get_sortby = $_GET['sortby'];
$get_direct = $_GET['direct'];
$get_group = $_GET['group'];
$get_cmd = $_GET['cmd'];
$get_name = $_GET['name'];
$get_gecos = $_GET['gecos'];
$get_nowdate = $_GET['nowdate'];
$get_year = $_GET['year'];
$get_month = $_GET['month'];
$get_day = $_GET['day'];
$get_year_end = $_GET['year_end'];
$get_month_end = $_GET['month_end'];
$get_day_end = $_GET['day_end'];
$get_smethod = $_GET['smethod'];
$get_begintime = $_GET['begintime'];
$session_domain = $_SESSION['G_DOMAIN'];
$Users = new users( $_SESSION['G_DOMAIN'] );
if ( $get_page == 0 )
{
    $get_page = 1;
}
$cur_direct = $get_direct;
if ( $get_direct == "down" )
{
    $sort_direct = "DESC";
    $get_direct = "up";
}
else
{
    $sort_direct = "ASC";
    $get_direct = "down";
}
switch ( $get_sortby )
{
    case "name" :
        $sortby = "ORDER BY pw_name";
        break;
    case "gecos" :
        $sortby = "ORDER BY pw_gecos";
        break;
    case "quota" :
        $sortby = "ORDER BY pw_shell";
        break;
    case "netdisk" :
        $sortby = "ORDER BY netdisk";
        break;
    case "group" :
        $sortby = "ORDER BY group_id";
        break;
    case "endtime" :
        $sortby = "ORDER BY expiration_time";
        break;
    case "time" :
    default :
        $sortby = "ORDER BY createtime";
        break;
}
if ( $cur_direct == "" )
{
    $sort_direct = "";
}
$sortby = $sortby." ".$sort_direct." ";
$sql = new mysql_class( );
$sql->connect( CFG_VPOPMAIL_DB );
$Users->getusergroup( );
$groupArray[0] = "&nbsp;";
foreach ( $Users->userGroup as $value )
{
    $groupArray[$value['id']] = $value['name'];
}
$totalquota = $Users->maxQuota;
$totaluser = $Users->maxUser;
if ( $totalquota != 0 )
{
    $totalrow = $Users->CurUserNum;
}
else if ( $totalquota == 0 )
{
    $totalrow = 0;
}
if ( $totalquota != 0 )
{
    $quota = intval( $Users->CurQuotaTotal );
}
else if ( $totalquota == 0 )
{
    $quota = 0;
}
$KeyStr = $managerStr;
$searchRows = $totalrow;
if ( $get_keyword != "" )
{
    $KeyStr .= "AND (pw_name LIKE '%{$get_keyword}%' or pw_gecos LIKE '%{$get_keyword}%')";
    if ( isset( $_GET['group'] ) && $_GET['group'] != "" )
    {
        $KeyStr .= " AND (group_id = {$get_group}) ";
    }
    $Users->searchrows( $KeyStr );
    $search_result = "<label>共有 <span>".$Users->searchRows."</span> 项结果符合搜索条件/label>";
}
if ( $get_cmd != "" && $get_cmd == "search" )
{
    if ( $get_year != "" && $get_month != "" && $get_day != "" )
    {
        if ( $get_month < 10 )
        {
            $get_month = "0{$get_month}";
        }
        if ( $get_day < 10 )
        {
            $get_day = "0{$get_day}";
        }
        $get_begintime = "".$get_year."".$get_month."".$get_day."000000";
    }
    if ( $get_year_end != "" && $get_month_end != "" && $get_day_end != "" )
    {
        if ( $get_month_end < 10 )
        {
            $get_month_end = "0{$get_month_end}";
        }
        if ( $get_day_end < 10 )
        {
            $get_day_end = "0{$get_day_end}";
        }
        $get_nowdate = "".$get_year_end."".$get_month_end."".$get_day_end."235959";
    }
    if ( $get_group != "" && isset( $get_group ) )
    {
        $KeyStr .= " AND group_id={$get_group} ";
    }
    if ( $get_name != "" )
    {
        $KeyStr .= " AND pw_nameKEYWORD1'KEYWORD2".$get_name."KEYWORD2'";
    }
    if ( $get_gecos != "" )
    {
        $KeyStr .= " AND pw_gecosKEYWORD1'KEYWORD2".$get_gecos."KEYWORD2'";
    }
    if ( $get_begintime != "" && $get_nowdate != "" )
    {
        $KeyStr .= " AND (createtime<{$get_nowdate} and createtime>{$get_begintime})";
    }
    if ( $get_begintime != "" && $get_nowdate == "" )
    {
        $KeyStr .= " AND (createtime>{$get_begintime})";
    }
    if ( $get_begintime == "" && $get_nowdate != "" )
    {
        $KeyStr .= " AND (createtime<{$get_nowdate})";
    }
    if ( $get_smethod == "a" )
    {
        $KeyStr = str_replace( "KEYWORD1", " LIKE ", $KeyStr );
        $KeyStr = str_replace( "KEYWORD2", "%", $KeyStr );
    }
    else if ( $get_smethod == "b" )
    {
        $KeyStr = str_replace( "KEYWORD1", "=", $KeyStr );
        $KeyStr = str_replace( "KEYWORD2", "", $KeyStr );
    }
    $Users->searchrows( $KeyStr );
    $search_result = "<label>{$LANG_DOMAIN_USER_SEARCH_RESULT} <span>".$Users->searchRows."</span> {$LANG_DOMAIN_USER_SEARCH_RESULT2}</label>";
}
$totalrow1 = $Users->searchRows == "" ? $totalrow : $Users->searchRows;
$totalpage = intval( $totalrow1 / CFG_NUMOFPAGE );
if ( $totalpage * CFG_NUMOFPAGE < $totalrow1 )
{
    ++$totalpage;
}
if ( $totalpage < $get_page )
{
    $get_page = $totalpage;
}
if ( $get_page == 0 )
{
    $get_page = 1;
}
$start_row = ( $get_page - 1 ) * CFG_NUMOFPAGE;
$prevpage = $get_page - 1;
$nextpage = $get_page + 1;
if ( $get_page <= 1 )
{
    $prevpage = 1;
}
if ( $totalpage <= $get_page )
{
    $nextpage = $totalpage;
}
$Users->getuserlist( $KeyStr, "{$sortby} LIMIT {$start_row}, ".CFG_NUMOFPAGE );
$List_Out = "";
$k = CFG_NUMOFPAGE * ( $get_page - 1 );
$netdisk_path = "{$CFG_NETDISK_PATH}/{$session_domain}/";
$nowtime = time( );
$nowtime = date( "YmdHis", $nowtime );
foreach ( $Users->listArr as $value )
{
    $value['expiration_time'] = timestamp2old( $value['expiration_time'] );
    $value['createtime'] = timestamp2old( $value['createtime'] );
    $limit = quotaparse( $value['pw_shell'] ); 
    //$quotaset = intval( $limit[quota] / 1048576 );
    $quotaset = intval(doubleval($value['pw_shell']) / 1048576 );
    $maxmsg = intval( $limit[maxmsg] );
    if ( $quotaset == 0 )
    {
        $quotaset = "-";
    }
    if ( $maxmsg == 0 )
    {
        $maxmsg = "-";
    }
    $homedir = $value[pw_dir];
    list( $curmaildirsize, $curmailnum ) = totalmaildirsize( $homedir );
    $curmaildirsize = intval( $curmaildirsize / 1048576 * 10 ) / 10;
    ++$k;
    $netdisk = $value['netdisk'];
    $username = $value['pw_name'];
    $func_gid = $value['func_gid'];
    $func_gid = $func_gid & FUNC_NETDISK;
    $netdisk_path1 = "{$netdisk_path}{$username}/";
    $ftpquota = new ftpquota( $netdisk_path1 );
    $totalsize = $ftpquota->fileTotalsize;
    $totalsize = intval( $totalsize / 1048576 * 10 + 0.5 ) / 10;
    if ( $value['expiration_time'] <= $nowtime && $value['expiration_time'] != 0 )
    {
        $tr_class = "class=\"overtime\"";
    }
    else
    {
        $tr_class = "class=\"tr2\"";
    }
    $List_Out .= "<tr {$tr_class} "."onmouseover=\"setPointer(this, 'over')\" onmouseout=\"setPointer(this, 'out')\" onmousedown=\"setPointer(this,'click')\">\n";
    if ( $value['pw_name'] == "postmaster" )
    {
        $List_Out .= "\t<td class=\"input\"><input type=\"checkbox\" name=ns[{$i}] value='".$value['pw_name']."' disabled /></td>\n";
    }
    else if ( $_SESSION['G_USERNAME'] != "postmaster" && $domain_privi->dopriv( SYSTEM_DELETE_MANAGE, "user_privi" ) && !$domain_privi->managermatch( $value['manager_id'] ) && !$domain_privi->dopriv( SYSTEM_GLOBAL_DELETE_MANAGE, "user_privi" ) )
    {
        $List_Out .= "\t<td class=\"input\"><input type=\"checkbox\" name=ns[{$i}] value='".$value['pw_name']."' disabled /></td>\n";
    }
    else if ( !$domain_privi->dopriv( SYSTEM_GLOBAL_DELETE_MANAGE, "user_privi" ) && !$domain_privi->dopriv( SYSTEM_DELETE_MANAGE, "user_privi" ) )
    {
        $List_Out .= "\t<td class=\"input\"><input type=\"checkbox\" name=ns[{$i}] value='".$value['pw_name']."' disabled /></td>\n";
    }
    else
    {
        $List_Out .= "\t<td class=\"input\"><input type=\"checkbox\" name=ns[{$i}] value='".$value['pw_name']."' /></td>\n";
    }
    $List_Out .= "\t<td class=\"num\" onmousedown=\"Flag({$i}+1)\">{$k}</td>\n";
    $monitor_pic = "&nbsp;";
    if ( $igenus_system['MONITOR'] == "enabled" && ( $value['func_gid'] & FUNC_MONITOR_IN || $value['func_gid'] & FUNC_MONITOR_OUT ) )
    {
        $monitor_pic = "<img src='images/surveillance.jpg' border='0' alt='MONITOR'>";
    }
    $List_Out .= "\t<td OnMouseDown='Flag({$i}+1)'>{$monitor_pic}</td>\n";
    $List_Out .= "\t<td class=\"user\" onmousedown='Flag({$i}+1)'><a href=\"useredit_form.php?cmd=modify&id=".$value['pw_id']."\">".$value['pw_name']."</a></td>\n";
    if ( $value['pw_name'] == "postmaster" )
    {
        $List_Out .= "\t<td class=\"name\" onmousedown=\"Flag({$i}+1)\">{$LANG_DOMAIN_USER_SYSADMIN}</td>\n";
    }
    else
    {
        $List_Out .= "\t<td class=\"name\" onmousedown=\"Flag({$i}+1)\">".$value['pw_gecos']."</td>\n";
    }
    $List_Out .= "\t<td class=\"group\" onmousedown=\"Flag({$i}+1)\">".$groupArray[$value['group_id']]."</td>\n";
    if ( $quotaset <= $curmaildirsize && $quotaset != "-" )
    {
        $List_Out .= "\t<td class=\"quota\" onmousedown=\"Flag({$i}+1)\"><label class=\"red\">{$quotaset}({$curmaildirsize})</label></td>\n";
    }
    else
    {
        $List_Out .= "\t<td class=\"quota\" onmousedown=\"Flag({$i}+1)\">{$quotaset}<span>({$curmaildirsize})</span></td>\n";
    }
    if ( $func_gid == 0 )
    {
        $List_Out .= "\t<td class=\"netdisk\" onmousedown=\"Flag({$i}+1)\">{$LANG_DOMAIN_USER_NO_PRIVI}</td>\n";
    }
    else if ( $netdisk <= $totalsize )
    {
        $List_Out .= "\t<td class=\"netdisk\" onmousedown=\"Flag({$i}+1)\"><label class=\"red\">{$netdisk}({$totalsize})</label></td>\n";
    }
    else
    {
        $List_Out .= "\t<td class=\"netdisk\" onmousedown=\"Flag({$i}+1)\">{$netdisk}<span>({$totalsize})</span></td>\n";
    }
    if ( $maxmsg <= $curmailnum && $maxmsg != "-" )
    {
        $List_Out .= "\t<td class=\"num2\" onmousedown=\"Flag({$i}+1)\"><label class=\"red\">{$maxmsg}({$curmailnum})</label></td>\n";
    }
    else
    {
        $List_Out .= "\t<td class=\"num2\" onmousedown=\"Flag({$i}+1)\">{$maxmsg}<span>({$curmailnum})</span></td>\n";
    }
    if ( $value['manager_id'] == 0 )
    {
        $manager_name = "postmaster";
    }
    else
    {
        $manager_name = $domain_privi->manager_arr[$value['manager_id']];
    }
    $List_Out .= "\t<TD class='date'>{$manager_name}</TD>\n";
    $date = substr( $value['createtime'], 0, 4 )."/".substr( $value['createtime'], 4, 2 )."/".substr( $value['createtime'], 6, 2 );
    if ( $value['expiration_time'] == 0 )
    {
        $enddate = $LANG_DOMAIN_USER_NO_LIMIT;
    }
    else if ( $value['expiration_time'] <= $nowtime )
    {
        $enddate = $LANG_DOMAIN_USER_END_TIME;
    }
    else
    {
        $enddate = substr( $value['expiration_time'], 0, 4 )."/".substr( $value['expiration_time'], 4, 2 )."/".substr( $value['expiration_time'], 6, 2 );
    }
    $List_Out .= "\t<td class=\"date\" onmousedown=\"Flag({$i}+1)\">{$date}</td>\n";
    $List_Out .= "\t<td class=\"enddate\" onmousedown=\"Flag({$i}+1)\">{$enddate}</td>\n";
    $access = $value['pw_gid'];
    $func_gid = $value['func_gid'];
    $accessStr = "";
    if ( !( $access & NO_SMTP ) )
    {
        $accessStr .= "<img src=\"images/t_01.jpg\" alt=\"ESMTP\" />";
    }
    else
    {
        $accessStr .= "<img src=\"images/f_01.jpg\" alt=\"ESMTP\" />";
    }
    if ( !( $access & NO_POP ) )
    {
        $accessStr .= " <img src=\"images/t_03.jpg\" alt=\"POP\" />";
    }
    else
    {
        $accessStr .= " <img src=\"images/f_03.jpg\" alt=\"POP\" />";
    }
    if ( !( $access & NO_IMAP ) )
    {
        $accessStr .= " <img src=\"images/t_05.jpg\" alt=\"IMAP\" />";
    }
    else
    {
        $accessStr .= " <img src=\"images/f_05.jpg\" alt=\"IMAP\" />";
    }
    if ( !( $access & NO_WEBMAIL ) )
    {
        $accessStr .= " <img src=\"images/t_07.jpg\" alt=\"WEBMAIL\" />";
    }
    else
    {
        $accessStr .= " <img src=\"images/f_07.jpg\" alt=\"WEBMAIL\" />";
    }
    if ( $func_gid & FUNC_NETDISK )
    {
        $accessStr .= " <img src=\"images/t_09.jpg\" alt=\"FILE\" />";
    }
    else
    {
        $accessStr .= " <img src=\"images/f_09.jpg\" alt=\"FILE\" />";
    }
    if ( $igenus_system['FTP'] == "enabled" )
    {
        if ( $func_gid & FUNC_NETDISK_FTP )
        {
            $accessStr .= " <img src=\"images/t_11.jpg\" alt=\"FTP\" />";
        }
        else
        {
            $accessStr .= " <img src=\"images/f_11.jpg\" alt=\"FTP\" />";
        }
    }
    $List_Out .= "\t<td class=\"popedom\">{$accessStr}</td>\n";
	//add by liuruit, disable uerdel----------------
    if ( $value['pw_name'] == "postmaster" )
    {
        //$List_Out .= "\t<td class=\"manage\"><span title=\"{$LANG_COMMON_LIST_NOT_DELETE}\" class=\"del\">{$LANG_COMMON_LIST_DELETE}</span>\n";
		$List_Out .= "\t<td class=\"manage\">\n";
    }
    else if ( $_SESSION['G_USERNAME'] != "postmaster" && $domain_privi->dopriv( SYSTEM_DELETE_MANAGE, "user_privi" ) && !$domain_privi->managermatch( $value['manager_id'] ) && !$domain_privi->dopriv( SYSTEM_GLOBAL_DELETE_MANAGE, "user_privi" ) )
    {
        //$List_Out .= "\t<td class=\"manage\"><span title=\"{$LANG_COMMON_LIST_NOT_DELETE}\" class=\"del\">{$LANG_COMMON_LIST_DELETE}</span>\n";
		$List_Out .= "\t<td class=\"manage\">\n";
    }
    else if ( !$domain_privi->dopriv( SYSTEM_GLOBAL_DELETE_MANAGE, "user_privi" ) && !$domain_privi->dopriv( SYSTEM_DELETE_MANAGE, "user_privi" ) )
    {
        //$List_Out .= "\t<td class=\"manage\"><span title=\"{$LANG_COMMON_LIST_NOT_DELETE}\" class=\"del\">{$LANG_COMMON_LIST_DELETE}</span>\n";
		$List_Out .= "\t<td class=\"manage\">\n";
    }
    else
    {
        //$List_Out .= "\t<td class=\"manage\"><a class=\"del\" title=\"{$LANG_COMMON_LIST_DELETE}\" href=\"#\" onclick=\"userdel('".$value['pw_name']."','".$value['pw_id']."');return false;\">{$LANG_COMMON_LIST_DELETE}</a>\n";
		$List_Out .= "\t<td class=\"manage\">\n";
    }
	//end here----------------------------------------
    if ( $_SESSION['G_USERNAME'] == "postmaster" )
    {
        if ( $value['pw_name'] == "postmaster" || $value['ismanager'] == 1 )
        {
            $List_Out .= "\t<span title=\"{$LANG_ADMIN_LOGIN_MANAGER}\" class=\"manager\">{$LANG_ADMIN_LOGIN_MANAGER}</span>\n";
        }
        else
        {
            $List_Out .= "\t<a class=\"manager\" title=\"{$LANG_ADMIN_MENU_MANAGER_ADD}\" href=\"manager.php?cmd=addform&username=".$value['pw_name']."\">{$LANG_ADMIN_LOGIN_MANAGER}</a>\n";
        }
    }
    $List_Out .= " <a class=\"modify\" title=\"{$LANG_COMMON_LIST_SETUP}\" href=\"useredit_form.php?cmd=modify&id=".$value['pw_id']."\">{$LANG_COMMON_LIST_SETUP}</a></td>\n";
    $List_Out .= "</tr>\n";
}
if ( $get_keyword != "" )
{
    $Out_Url = "&keyword={$get_keyword}";
}
if ( $get_manager != "" )
{
    $Out_Url = "&manager={$get_manager}";
}
if ( $get_cmd != "" )
{
    $Out_Url .= "&cmd={$get_cmd}";
}
if ( $get_group != "" )
{
    $Out_Url .= "&group={$get_group}";
}
if ( $get_name != "" )
{
    $Out_Url .= "&name={$get_name}";
}
if ( $get_gecos != "" )
{
    $Out_Url .= "&gecos={$get_gecos}";
}
if ( $get_begintime != "" && $get_nowdate != "" )
{
    $Out_Url .= "&nowdate={$get_nowdate}";
    $Out_Url .= "&begintime={$get_begintime}";
}
if ( $get_begintime != "" && $get_nowdate == "" )
{
    $Out_Url .= "&begintime={$get_begintime}";
}
if ( $get_begintime == "" && $get_nowdate != "" )
{
    $Out_Url .= "&nowdate={$get_nowdate}";
}
if ( $get_smethod != "" )
{
    $Out_Url .= "&smethod={$get_smethod}";
}
if ( $domain_privi->dopriv( SYSTEM_GLOBAL_MANAGE, "user_privi" ) )
{
    foreach ( $domain_privi->manager_arr as $key => $value )
    {
        $SELECTED = "";
        if ( $get_manager == $key )
        {
            $SELECTED = "selected";
        }
        $ManagerSelect .= "<option value=\"{$key}\" {$SELECTED}>{$value}</option>\n";
    }
}
$template = new template( CFG_TEMPLATE_PATH."domainAdmin/" );
$template->set_file( array( "page" => "list.html", "copyright" => "copyright_bottom.html" ) );
$template->set_var( array( "IGENUS_TITLE" => $CFG_IGENUS_ADM, "DOMAIN" => $session_domain, "USED_QUOTA" => $quota, "TOTAL_QUOTA" => $totalquota, "QUOTA_SPARE" => $totalquota - $quota < 0 ? 0 : $totalquota - $quota, "USED_USER" => $totalrow, "TOTAL_USER" => $totaluser, "MANAGERSELECT" => $ManagerSelect, "MANAGER_MEN" => $domain_privi->dopriv( SYSTEM_GLOBAL_MANAGE, "user_privi" ) ? "" : "<!--", "MANAGER_MEN_BY" => $domain_privi->dopriv( SYSTEM_GLOBAL_MANAGE, "user_privi" ) ? "" : "//-->", "SHOW_ALL_MEN" => $get_keyword != "" || $get_manager != "" ? "" : "<!--", "SHOW_ALL_MEN_BY" => $get_keyword != "" || $get_manager != "" ? "" : "//-->", "USER_SPARE" => $totaluser - $totalrow < 0 ? 0 : $totaluser - $totalrow, "PAGE_INDEX" => pageindexadmin( CFG_NUMOFPAGE, $searchRows, $get_page, "sortby={$get_sortby}&direct={$cur_direct}{$Out_Url}" ), "HREF_USER" => "?sortby=name&page={$get_page}&direct={$get_direct}{$Out_Url}", "HREF_NAME" => "?sortby=gecos&page={$get_page}&direct={$get_direct}{$Out_Url}", "HREF_GROUP" => "?sortby=group&page={$get_page}&direct={$get_direct}{$Out_Url}", "HREF_QUOTA" => "?sortby=quota&page={$get_page}&direct={$get_direct}{$Out_Url}", "HREF_NETDISK" => "?sortby=netdisk&page={$get_page}&direct={$get_direct}{$Out_Url}", "HREF_DATE" => "?sortby=time&page={$get_page}&direct={$get_direct}{$Out_Url}", "HREF_ENDDATE" => "?sortby=endtime&page={$get_page}&direct={$get_direct}{$Out_Url}", "LIST_OUT" => $List_Out, "HREF_EXIT" => isset( $_SESSION['G_SYSADMID'] ) ? "javascript:window.close();" : "logout.php", "KEYWORD" => isset( $_GET['keyword'] ) ? $_GET['keyword'] : $LANG_COMMON_LIST_KEYWORD, "SEARCH_RESULT" => $search_result, "LANG_DOMAIN_MENU_USER" => $LANG_DOMAIN_MENU_USER, "LANG_DOMAIN_MENU_USER_LIST" => $LANG_DOMAIN_MENU_USER_LIST, "LANG_COMMON_LIST_PAGE" => $LANG_COMMON_LIST_PAGE, "LANG_DOMAIN_USER_QUOTA" => $LANG_DOMAIN_USER_QUOTA, "LANG_DOMAIN_USER_CUR_QUOTA" => $LANG_DOMAIN_USER_CUR_QUOTA, "LANG_DOMAIN_USER_SUR_QUOTA" => $LANG_DOMAIN_USER_SUR_QUOTA, "LANG_DOMAIN_USER_TOTAL" => $LANG_DOMAIN_USER_TOTAL, "LANG_DOMAIN_USER_CUR_TOTAL" => $LANG_DOMAIN_USER_CUR_TOTAL, "LANG_DOMAIN_USER_SUR_TOTAL" => $LANG_DOMAIN_USER_SUR_TOTAL, "LANG_COMMON_LIST_MUMBER" => $LANG_COMMON_LIST_MUMBER, "LANG_DOMAIN_USER_USER" => $LANG_DOMAIN_USER_USER, "LANG_DOMAIN_USER_NAME" => $LANG_DOMAIN_USER_NAME, "LANG_DOMAIN_USER_GROUP" => $LANG_DOMAIN_USER_GROUP, "LANG_DOMAIN_USER_USER_QUOTA" => $LANG_DOMAIN_USER_USER_QUOTA, "LANG_DOMAIN_USER_USER_NETDISK" => $LANG_DOMAIN_USER_USER_NETDISK, "LANG_DOMAIN_USER_MSG" => $LANG_DOMAIN_USER_MSG, "LANG_ADMIN_DOMAIN_LIST_USER_CREATE" => $LANG_ADMIN_DOMAIN_LIST_USER_CREATE, "LANG_DOMAIN_USER_CREATEDATE" => $LANG_DOMAIN_USER_CREATEDATE, "LANG_DOMAIN_USER_ENDDATE" => $LANG_DOMAIN_USER_ENDDATE, "LANG_DOMAIN_USER_PRIVI" => $LANG_DOMAIN_USER_PRIVI, "LANG_DOMAIN_USER_MANAGE" => $LANG_DOMAIN_USER_MANAGE, "LANG_DOMAIN_USER_SELECT_ALL" => $LANG_DOMAIN_USER_SELECT_ALL, "LANG_DOMAIN_USER_DELETE" => $LANG_COMMON_LIST_DELETE, "LANG_DOMAIN_USER_SEARCH" => $LANG_DOMAIN_USER_SEARCH, "LANG_DOMAIN_USER_SHOW_ALL" => $LANG_DOMAIN_USER_SHOW_ALL, "LANG_DOMAIN_MENU_USER_ADD" => $LANG_DOMAIN_MENU_USER_ADD, "LANG_DOMAIN_DELELT_USER" => $LANG_DOMAIN_DELELT_USER, "LANG_DOMAIN_DELELT_SELECT_USER" => $LANG_DOMAIN_DELELT_SELECT_USER, "LANG_DOMAIN_SELECT_USER" => $LANG_DOMAIN_SELECT_USER, "LANG_COMMON_LIST_KEYWORD" => $LANG_COMMON_LIST_KEYWORD, "LANG_COMMON_LIST_KEYWORD" => $LANG_COMMON_LIST_KEYWORD, "LANG_LIST_KEYWORD_ALERT" => $LANG_COMMON_ALERT.$LANG_COMMON_LIST_KEYWORD, "LANG_ADMIN_DOMAIN_LIST_MANAGER_SELECT" => $LANG_ADMIN_DOMAIN_LIST_MANAGER_SELECT ,"LANG_POLICY_SETTINGS"=>$LANG_POLICY_SETTINGS) );
$template->parse( "COPYRIGHT", "copyright" );
$template->parse( "out", "page" );
$template->p( "out" );
exit( );
?>
