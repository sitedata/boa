#!/usr/bin/perl

### TODO - rewrite this legacy script in bash

###
### System Services Monitor running every 5 seconds
###
if (!-d "/var/run/mysqld") {
  system("mkdir -p /var/run/mysqld");
  system("chown -R mysql:root /var/run/mysqld");
}
&cpu_count_load;
&global_action;
foreach $USER (sort keys %li_cnt) {
  print " $li_cnt{$USER}\t$USER\n";
  push(@donetable," $li_cnt{$USER}\t$USER");
  $sumar = $sumar + $li_cnt{$USER};
  if ($USER eq "mysql") {$mysqlives = "YES"; $mysqlsumar = $li_cnt{$USER};}
  if ($USER eq "jetty7") {$jetty7lives = "YES"; $jetty7sumar = $li_cnt{$USER};}
  if ($USER eq "jetty8") {$jetty8lives = "YES"; $jetty8sumar = $li_cnt{$USER};}
  if ($USER eq "jetty9") {$jetty9lives = "YES"; $jetty9sumar = $li_cnt{$USER};}
  if ($USER eq "solr7") {$solr7lives = "YES"; $solr7sumar = $li_cnt{$USER};}
}
foreach $COMMAND (sort keys %li_cnt) {
  if ($COMMAND =~ /named/) {$namedlives = "YES"; $namedsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /buagent/) {$buagentlives = "YES"; $buagentsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /collectd/) {$collectdlives = "YES"; $collectdsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /dhcpcd-bin/) {$dhcpcdlives = "YES"; $dhcpcdsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /nginx/) {$nginxlives = "YES"; $nginxsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /pdnsd/) {$pdnsdlives = "YES"; $pdnsdsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /php-cgi/) {$phplives = "YES"; $phpsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /php-fpm/) {$fpmlives = "YES"; $fpmsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /postfix/) {$postfixlives = "YES"; $postfixsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /pure-ftpd/) {$ftplives = "YES"; $ftpsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /redis-server/) {$redislives = "YES"; $redissumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /newrelic-daemon/) {$newrelicdaemonlives = "YES"; $newrelicdaemonsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /nrsysmond/) {$newrelicsysmondlives = "YES"; $newrelicsysmondsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /rsyslogd/) {$rsyslogdlives = "YES"; $rsyslogdsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /sbin\/syslogd/ && -f "/var/run/syslogd.pid") {$sysklogdlives = "YES"; $sysklogdsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /sbin\/syslogd/ && -f "/var/run/syslog.pid") {$syslogdlives = "YES"; $syslogdsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /xinetd/) {$xinetdlives = "YES"; $xinetdsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /lsyncd/) {$lsyncdlives = "YES"; $lsyncdsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /sshd/) {$sshdlives = "YES"; $sshdsumar = $li_cnt{$COMMAND};}
  if ($COMMAND =~ /proxysql/) {$pxydlives = "YES"; $pxydsumar = $li_cnt{$COMMAND};}
}
foreach $X (sort keys %li_cnt) {
  if ($X =~ /php81/) {$php81lives = "YES";}
  if ($X =~ /php80/) {$php80lives = "YES";}
  if ($X =~ /php74/) {$php74lives = "YES";}
  if ($X =~ /php73/) {$php73lives = "YES";}
  if ($X =~ /php72/) {$php72lives = "YES";}
  if ($X =~ /php71/) {$php71lives = "YES";}
  if ($X =~ /php70/) {$php70lives = "YES";}
  if ($X =~ /php56/) {$php56lives = "YES";}
}
foreach $K (sort keys %li_cnt) {
  if ($K =~ /convert/) {$convertlives = "YES"; $convertsumar = $li_cnt{$K};}
}
if ($convertsumar > 1)
{
  &convert_action;
}
print "\n $sumar ALL procs\t\tGLOBAL";
print "\n $namedsumar Bind procs\t\tGLOBAL" if ($namedlives);
print "\n $buagentsumar Backup procs\t\tGLOBAL" if ($buagentlives);
print "\n $collectdsumar Collectd\t\tGLOBAL" if ($collectdlives);
print "\n $dhcpcdsumar dhcpcd procs\t\tGLOBAL" if ($dhcpcdlives);
print "\n $fpmsumar FPM procs\t\tGLOBAL" if ($fpmlives);
print "\n 1 FPM81 procs\t\tGLOBAL" if ($php81lives);
print "\n 1 FPM80 procs\t\tGLOBAL" if ($php80lives);
print "\n 1 FPM74 procs\t\tGLOBAL" if ($php74lives);
print "\n 1 FPM73 procs\t\tGLOBAL" if ($php73lives);
print "\n 1 FPM72 procs\t\tGLOBAL" if ($php72lives);
print "\n 1 FPM71 procs\t\tGLOBAL" if ($php71lives);
print "\n 1 FPM70 procs\t\tGLOBAL" if ($php70lives);
print "\n 1 FPM56 procs\t\tGLOBAL" if ($php56lives);
print "\n $ftpsumar FTP procs\t\tGLOBAL" if ($ftplives);
print "\n $mysqlsumar MySQL procs\t\tGLOBAL" if ($mysqlives);
print "\n $nginxsumar Nginx procs\t\tGLOBAL" if ($nginxlives);
print "\n $pdnsdsumar DNS procs\t\tGLOBAL" if ($pdnsdlives);
print "\n $phpsumar PHP procs\t\tGLOBAL" if ($phplives);
print "\n $postfixsumar Postfix procs\tGLOBAL" if ($postfixlives);
print "\n $redissumar Redis procs\t\tGLOBAL" if ($redislives);
print "\n $newrelicdaemonsumar New Relic Apps\tGLOBAL" if ($newrelicdaemonlives);
print "\n $newrelicsysmondsumar New Relic Server\tGLOBAL" if ($newrelicsysmondlives);
print "\n $jetty7sumar Jetty7 procs\t\tGLOBAL" if ($jetty7lives);
print "\n $jetty8sumar Jetty8 procs\t\tGLOBAL" if ($jetty8lives);
print "\n $jetty9sumar Jetty9 procs\t\tGLOBAL" if ($jetty9lives);
print "\n $solr7sumar Solr7 procs\t\tGLOBAL" if ($solr7lives);
print "\n $rsyslogdsumar Syslog procs\t\tGLOBAL" if ($rsyslogdlives);
print "\n $sysklogdsumar Syslog procs\t\tGLOBAL" if ($sysklogdlives);
print "\n $syslogdsumar Syslog procs\t\tGLOBAL" if ($syslogdlives);
print "\n $convertsumar Convert procs\t\tGLOBAL" if ($convertlives);
print "\n $xinetdsumar Xinetd procs\t\tGLOBAL" if ($xinetdlives);
print "\n $lsyncdsumar Lsyncd procs\t\tGLOBAL" if ($lsyncdlives);
print "\n $sshdsumar SSHd procs\t\tGLOBAL" if ($sshdlives);
print "\n $pxydsumar PxySQL procs\t\tGLOBAL" if ($pxydlives);
print "\n";

system("service bind9 restart") if (!$namedsumar && -f "/etc/init.d/bind9");
system("service ssh restart") if (!$sshdsumar && -f "/etc/init.d/ssh");
system("service proxysql restart") if (!$pxydsumar && -f "/etc/init.d/proxysql");

if (-e "/usr/sbin/pdnsd" && (!$pdnsdsumar || !-e "/etc/resolvconf/run/interface/lo.pdnsd")) {
  system("mkdir -p /var/cache/pdnsd");
  system("chown -R pdnsd:proxy /var/cache/pdnsd");
  system("resolvconf -u");
  system("service pdnsd restart");
  system("pdnsd-ctl empty-cache");
}

if ((!$mysqlsumar || $mysqlsumar > 150) && !-f "/var/run/mysql_restart_running.pid" && !-f "/var/run/boa_run.pid" && !-f "/root/.remote.db.cnf") {
  system("bash /var/xdrago/move_sql.sh");
}

if (-f "/root/.mstr.clstr.cnf" || -f "/root/.wbhd.clstr.cnf") {
  if ($mysqlives && -f "/root/.remote.db.cnf") {
    system("mysql -u root -e \"SET GLOBAL innodb_max_dirty_pages_pct = 0\;\"");
    system("mysql -u root -e \"SET GLOBAL innodb_change_buffering = \'none\'\;\"");
    system("mysql -u root -e \"SET GLOBAL innodb_buffer_pool_dump_at_shutdown = 1\;\"");
    system("mysql -u root -e \"SET GLOBAL innodb_io_capacity = 2000\;\"");
    system("mysql -u root -e \"SET GLOBAL innodb_io_capacity_max = 4000\;\"");
    system("mysql -u root -e \"SET GLOBAL innodb_buffer_pool_dump_pct = 100\;\"");
    system("mysql -u root -e \"SET GLOBAL innodb_buffer_pool_dump_now = ON\;\"");
    system("service mysql stop");
  }
}

if (!-f "/root/.dbhd.clstr.cnf") {
  if (!-d "/var/run/redis") {
    system("mkdir -p /var/run/redis");
    system("chown -R redis:redis /var/run/redis");
  }
  if (!$redissumar && (-f "/etc/init.d/redis-server" || -f "/etc/init.d/redis")) {
    if (-f "/etc/init.d/redis-server") { system("service redis-server start"); }
    elsif (-f "/etc/init.d/redis") { system("service redis start"); }
  }
  local(@RSARR)=`grep -e redis_client_socket /data/conf/global.inc`;
  foreach $line (@RSARR) {
    if ($line =~ /redis_client_socket/) {$redissocket = "YES";}
  }
  system("service redis-server restart") if (!-e "/var/run/redis/redis.sock" && $redissocket);
  sleep(2);
  system("service redis-server restart") if (!-f "/var/run/redis/redis.pid");
}

system("service newrelic-daemon restart") if (!$newrelicdaemonsumar && -f "/etc/init.d/newrelic-daemon");
system("service newrelic-sysmond restart") if (!$newrelicsysmondsumar && -f "/etc/init.d/newrelic-sysmond" && -f "/root/.enable.newrelic.sysmond.cnf");
system("service newrelic-sysmond stop") if ($newrelicsysmondsumar && -f "/etc/init.d/newrelic-sysmond" && !-f "/root/.enable.newrelic.sysmond.cnf");
system("service postfix restart") if (!$postfixsumar && -f "/etc/init.d/postfix");

if (!$nginxsumar && -f "/etc/init.d/nginx" && !-f "/root/.dbhd.clstr.cnf") {
  system("killall -9 nginx");
  system("service nginx start");
}

if ($nginxsumar) {
  if (-f "/root/.dbhd.clstr.cnf") {
    if (!-f "/root/.mstr.clstr.cnf" && !-f "/root/.wbhd.clstr.cnf") {
      system("killall -9 nginx");
    }
  }
}

if (-f "/root/.dbhd.clstr.cnf") {
  if ($php81lives || $php80lives || $php74lives || $php73lives || $php72lives || $php71lives || $php70lives || $php56lives) {
    system("killall -9 php-fpm");
  }
  if ($redislives) {
    system("killall -9 redis-server");
  }
}
else {
  system("service php81-fpm restart") if ((!$php81lives || !$fpmsumar || $fpmsumar > 8 || !-f "/var/run/php81-fpm.pid") && -f "/etc/init.d/php81-fpm");
  system("service php80-fpm restart") if ((!$php80lives || !$fpmsumar || $fpmsumar > 8 || !-f "/var/run/php80-fpm.pid") && -f "/etc/init.d/php80-fpm");
  system("service php74-fpm restart") if ((!$php74lives || !$fpmsumar || $fpmsumar > 8 || !-f "/var/run/php74-fpm.pid") && -f "/etc/init.d/php74-fpm");
  system("service php73-fpm restart") if ((!$php73lives || !$fpmsumar || $fpmsumar > 8 || !-f "/var/run/php73-fpm.pid") && -f "/etc/init.d/php73-fpm");
  system("service php72-fpm restart") if ((!$php72lives || !$fpmsumar || $fpmsumar > 8 || !-f "/var/run/php72-fpm.pid") && -f "/etc/init.d/php72-fpm");
  system("service php71-fpm restart") if ((!$php71lives || !$fpmsumar || $fpmsumar > 8 || !-f "/var/run/php71-fpm.pid") && -f "/etc/init.d/php71-fpm");
  system("service php70-fpm restart") if ((!$php70lives || !$fpmsumar || $fpmsumar > 8 || !-f "/var/run/php70-fpm.pid") && -f "/etc/init.d/php70-fpm");
  system("service php56-fpm restart") if ((!$php56lives || !$fpmsumar || $fpmsumar > 8 || !-f "/var/run/php56-fpm.pid") && -f "/etc/init.d/php56-fpm");
}

system("service jetty7 start") if (!$jetty7sumar && -f "/etc/init.d/jetty7");
system("service jetty8 start") if (!$jetty8sumar && -f "/etc/init.d/jetty8");
system("service jetty9 start") if (!$jetty9sumar && -f "/etc/init.d/jetty9");
system("service solr7 start") if (!$solr7sumar && -f "/etc/init.d/solr7");
system("service collectd start") if (!$collectdsumar && -f "/etc/init.d/collectd");
system("service xinetd start") if (!$xinetdsumar && -f "/etc/init.d/xinetd");
system("service lsyncd start") if (!$lsyncdsumar && -f "/etc/init.d/lsyncd");
system("service postfix restart") if (!-f "/var/spool/postfix/pid/master.pid");

if (-f "/usr/local/sbin/pure-config.pl") {
  if (-f "/root/.mstr.clstr.cnf" || -f "/root/.wbhd.clstr.cnf" || -f "/root/.dbhd.clstr.cnf") {
    if ($ftpsumar) {
      system("killall -9 pure-ftpd");
    }
  }
  else {
    `/usr/local/sbin/pure-config.pl /usr/local/etc/pure-ftpd.conf` if (!$ftpsumar);
  }
}

if ($mysqlsumar > 0 ) {
 `mysqladmin flush-hosts &> /dev/null`;
  print "\n MySQL hosts flushed...\n";
}
if ($dhcpcdlives) {
  $thishostname=`cat /etc/hostname`;
  chomp($thishostname);
  system("hostname -b $thishostname");
}
if (-f "/etc/init.d/rsyslog") {
  if (!$rsyslogdsumar || !-f "/var/run/rsyslogd.pid") {
    system("killall -9 rsyslogd");
    system("service rsyslog restart");
  }
}
elsif (-f "/etc/init.d/sysklogd") {
  if (!$sysklogdsumar || !-f "/var/run/syslogd.pid") {
    system("killall -9 sysklogd");
    system("service sysklogd restart");
  }
}
elsif (-f "/etc/init.d/inetutils-syslogd") {
  if (!$syslogdsumar || !-f "/var/run/syslog.pid") {
    system("killall -9 syslogd");
    system("service inetutils-syslogd restart");
  }
}
exit;

#############################################################################
sub global_action
{
  local(@MYARR)=`ps auxf 2>&1`;
  foreach $line (@MYARR) {
    $line =~ s/[^a-zA-Z0-9\:\s\t\/\-\@\_\(\)\*\[\]\.\,\?\=\|\\\+]//g;
    local($USER, $PID, $CPU, $MEM, $VSZ, $RSS, $TTY, $STAT, $START, ${TIME}, $COMMAND, $B, $K, $X, $Y, $Z, $T) = split(/\s+/,$line);
    $PID =~ s/[^0-9]//g;
    $li_cnt{$USER}++ if ($PID);
    $li_cnt{$X}++ if ($PID && $COMMAND =~ /php-fpm/ && $X =~ /php/);
    $li_cnt{$K}++ if ($PID && $COMMAND =~ /^(\|)/ && $K =~ /convert/);
    local($fpm_result) = "CTRL";

    if ($PID)
    {
      local($HOUR, $MIN) = split(/:/,${TIME});
      $MIN =~ s/^0//g;
      if ($COMMAND !~ /^(\\)/ && $COMMAND !~ /^(\|)/)
      {
        if ($COMMAND =~ /nginx/) {
          if ($USER =~ /root/) {
            $li_cnt{$COMMAND}++;
          }
        }
        elsif ($COMMAND =~ /sendmail/) {
          if ($USER =~ /root/) {
            system("kill -9 $PID");
          }
        }
        else {
          if ($COMMAND !~ /java/) {
            $li_cnt{$COMMAND}++;
          }
        }
      }
    }
  }
}

#############################################################################
sub convert_action
{
  local(@MYARR)=`ps auxf 2>&1`;
  foreach $line (@MYARR) {
    $line =~ s/[^a-zA-Z0-9\:\s\t\/\-\@\_\(\)\*\[\]\.\,\?\=\|\\\+]//g;
    local($USER, $PID, $CPU, $MEM, $VSZ, $RSS, $TTY, $STAT, $START, ${TIME}, $COMMAND, $B, $K, $X, $Y, $Z, $T) = split(/\s+/,$line);
    $PID =~ s/[^0-9]//g;
    if ($PID)
    {
      local($HOUR, $MIN) = split(/:/,${TIME});
      $MIN =~ s/^0//g;
      if ($COMMAND =~ /^(\|)/ && $K =~ /convert/ && $CPU > 10 && $MIN > 1 && ($STAT =~ /R/ || $STAT =~ /Z/))
      {
        $timedate=`date +%y%m%d-%H%M%S`;
        chomp($timedate);
        if ($convertsumar > 5 && $CPU > 50) {
          system("kill -9 $PID");
         `echo "$USER $CPU $STAT $START ${TIME} $timedate KILL Q $convertsumar" >> /var/xdrago/log/convert.kill.log`;
          $kill_convert = "YES";
        }
        else {
         `echo "$USER $CPU $STAT $START ${TIME} $timedate WATCH $convertsumar" >> /var/xdrago/log/convert.watch.log`;
        }
      }

      if ($kill_convert && $COMMAND =~ /^(\|)/ && $K =~ /bin/ && $Y =~ /convert/)
      {
        system("kill -9 $PID");
       `echo "$USER $CPU $STAT $START ${TIME} $timedate KILL Z $convertsumar" >> /var/xdrago/log/convert.kill.log`;
      }
    }
  }
}

#############################################################################
sub cpu_count_load
{
  local($PROCS)=`grep -c processor /proc/cpuinfo`;
  chomp($PROCS);
  $MAXSQLCPU = $PROCS."00";
  $MAXFPMCPU = $PROCS."00";
  if ($PROCS > 2)
  {
    $MAXSQLCPU = 600;
  }
  $MAXSQLCPU = $MAXSQLCPU - 5;
  $MAXFPMCPU = $MAXFPMCPU - 5;
}
###EOF2023###
