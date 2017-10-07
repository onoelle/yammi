#
# Regular cron jobs for the yammi package
#
0 4	* * *	root	[ -x /usr/bin/yammi_maintenance ] && /usr/bin/yammi_maintenance
