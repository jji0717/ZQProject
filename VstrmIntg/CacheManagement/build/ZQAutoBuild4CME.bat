@rem = '-*- Perl -*-';
@rem = '
@echo off
perl -S %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto EndOfPerl
';
# -----------------------------------------------------------------
# File Name:     ZQAutoBuild4CME.bat
# Author:        Interactive ZQ
# Security:      Tech Mgm
# Description:   This program build defined project automaticall, 
#                and email report to attns
# Modification Log:
# -----------------------------------------------------------
# 2005-03-24  0.1 Hui.Shao  Create
# 2014-4-9    0.2 Ken Qian  Update 4 CME Build
# -----------------------------------------------------------------

use Getopt::Long;
use Win32;
use File::Copy;
use Cwd;

# -----------------
# global declares
# -----------------
use vars qw(
	$VSSROOT $VSSUSER $VSSPASSWD $VSSSERVER $SOSDATABASE
	$BUILD_SETTINGS $BUILD_ROOT $RELEASE_ROOT %BUILD_BATCH
	$SSCMD $MSDEV6CMD $MSDEV7CMD $EMAILATTNS
	);

#flags
my ($CRTCLBATCH, $NIGHTLY, $GETSOURCE, $COMMITBUILD2SOURCE, $PUBLISHBUILD, $EMAILNOTIF)
=  (0, 0, 1, 1, 1, 0);

my $LOGFILE;

use Env qw(SSDIR CMEPATH TEMP ITVSDKPATH BuildVerNum);

my @SRC_BRANCH_STACK;
my @OUTPUT_STACK;
my $LOCAL_BUILD_BATCH_FN;

my $BUILD_OPTS_DESC;
my $BUILD_PRJS_DESC;

require "profiles.pl";

use vars qw(@ALL_PROJS @NIGHLY_PROJLIST);

my @PROJLIST = ();

%BUILD_BATCH = (
	'batch_num' => 0,
	'batch_fn'  => 'buildbatch.txt',
	'batch_vn'  => 'batchnum.txt',
	'failed'    => 0,
	'crnt_step' => 'initialization',
	'comp_owner' => $EMAILATTNS, 
);

# ------------------
# main procedure 
# ------------------

#read arguments
if ($ARGV[0])
{
	&GetOptions(
				'help'     =>		\$opt_help,
				'list'     =>		\$opt_list,
               'nightly'    =>      \$opt_nightly,
               'buildroot=s' =>     \$opt_buildroot,
               'releaseroot=s' =>     \$opt_releaseroot,
               'vodsdk=s'   => 		\$opt_vodsdk,
               'projects=s'     => 	\$opt_projname,
			   
               'vssroot=s' =>       \$opt_vssroot,
               'vssuser=s' =>       \$opt_vssuser,
               'vsspasswd=s' =>     \$opt_vsspasswd,

               'skipgetsource' =>   \$opt_skipgetsource,

               'emailreport' =>     \$opt_emailreport,
               'emailto=s' =>       \$opt_emailto,
               'label=s'   =>	    \$opt_label,
               'updateversion'  =>	\$opt_updateV,
			   
			   'logfile=s' =>		\$opt_logfile,
               'test'   => 			\$opt_test,
		);
		
		$NIGHTLY     = 1				if $opt_nightly;

		$BUILD_ROOT  = fixPathToOS($opt_buildroot) 	if $opt_buildroot;
		$ITVSDKPATH =  fixPathToOS($opt_vodsdk) 		if $opt_vodsdk;
		
		$RELEASE_ROOT = fixPathToOS($opt_releaseroot) if $opt_releaseroot;
		
		$VSSROOT = fixPathToOS($opt_vssroot)			if $opt_vssroot;
		$VSSUSER = $opt_vssuser			if $opt_vssuser;
		$VSSPASSWD = $opt_vsspasswd		if $opt_vsspasswd;
		
		$GETSOURCE = 0					if $opt_skipgetsource;
		$EMAILATTNS = $opt_emailto		if $opt_emailto;
		$EMAILNOTIF = 1					if $opt_emailreport and $EMAILATTNS;
		
		$COMMITBUILD2SOURCE  = 0		if $opt_test;
		$CRTCLBATCH = 0					if $opt_test;
		$EMAILNOTIF = 0					if $opt_test;

		$LOGFILE  =fixPathToOS($opt_logfile) 	if $opt_logfile;
}

usage() if $opt_help;
if ($opt_list)
{
	print "List all project profiles:\n";
	foreach $proj (@ALL_PROJS)
	{
		print "  [$proj->{name}] $proj->{desc}\n";
	}
	print "\n";
	exit 0;
}

my $labelinfo="build batch# "."$opt_label" if($opt_label);
my @stamp = (localtime(time))[0,1,2,3,4,5];
my $BUILD_TIME = sprintf "%04d-%02d-%02d %02d:%02d", $stamp[5]+1900, $stamp[4]+1, $stamp[3], $stamp[2], $stamp[1], $stamp[0];

$SSDIR = $VSSROOT;
$CMEPATH = fixPathToOS($BUILD_ROOT."/CacheManagement");
$EMAILCMD = fixPathToOS($BUILD_ROOT."/CacheManagement/build/utils/blat.exe");
$MD5SUM = fixPathToOS($BUILD_ROOT."/CacheManagement/build/utils/md5sum.exe -b ");

$EMAILNOTIF = 1                         	if $NIGHTLY and $EMAILATTNS;
$LOGFILE = fixPathToOS($TEMP."/ZQABNight.log")  	if $NIGHTLY and !$LOGFILE;
$LOGFILE = fixPathToOS($TEMP."/ZQAutoBuild.log")	if $EMAILNOTIF and !$LOGFILE;

if ($NIGHTLY)
{
	$GETSOURCE = 1;
	$COMMITBUILD2SOURCE = 1;
	$CRTCLBATCH =1;
	$PUBLISHBUILD = 1;
	open TMP, "> $LOGFILE";
	close TMP;
	@PROJLIST = @NIGHLY_PROJLIST;
        &getdependence();
}

if (!$NIGHTLY)
{
	Die("A manual build must specify a project by using --project=\"<PN>[,<PN>...]\"") if !$opt_projname;
	my @projnames = split(/,/, $opt_projname);
	@PROJLIST = ();
	my $pn;
	foreach $pn (@projnames)
	{
		foreach $proj (@ALL_PROJS)
		{
  			push (@PROJLIST, $proj) if trim($pn) eq trim($proj->{name});
  		}
  	}

	print "********Get project dependence********\n";
	die("A manual build must specify a valid project name by using --projects=<PN>[,<PN>...]") if !@PROJLIST;
	&getdependence();
}


&log("********** AutoBuild starts **********");
{
my $proj;
$BUILD_OPTS_DESC  = "**\tProjects to build: \"";
foreach $proj (@PROJLIST)
{
	$BUILD_OPTS_DESC .= $proj->{name}.",";
}
$BUILD_OPTS_DESC .= "\"\n";
}
$BUILD_OPTS_DESC .= "**\tOptions:\n";
$BUILD_OPTS_DESC .= "\tVSS          = $VSSUSER:****\@$SSDIR\n";
$BUILD_OPTS_DESC .= "\tBuildRoot    = $BUILD_ROOT\n";
$BUILD_OPTS_DESC .= "\tGetSource    = $GETSOURCE\n";
$BUILD_OPTS_DESC .= "\tEmails       = $EMAILATTNS\n"      if $EMAILNOTIF;
$BUILD_OPTS_DESC .= "\tCommitBuilds = $COMMITBUILD2SOURCE\n";
$BUILD_OPTS_DESC .= "\tRelease      = $RELEASE_ROOT\n"    if $PUBLISHBUILD;

my $envs= "**\tEnvironment Variables:\n";
{
	open ENV_F, "set | ";
	my $line;
	while ($line = <ENV_F>)
	{
		$envs .= "\t".$line;
	}
	close ENV_F;
}
&log($BUILD_OPTS_DESC.$envs);

if ($opt_updateV)
{
foreach $proj (@PROJLIST)
{
	&renewBuildNumber();
	$BuildVerNum = $proj->{prod_ver}.".$proj->{build_num}";
    updateVersionH($proj);
    updateVersionI($proj);
}
exit;
}

&getSource() if $GETSOURCE and !$BUILD_BATCH{failed};

&renewBuildNumber() if  !$BUILD_BATCH{failed};

@OUTPUT_STACK = [];

foreach $proj (@PROJLIST)
{
  if ($proj->{xml_path})
  {
  	my $xml_path = $proj->{xml_path};
  	foreach $xmlpath (@$xml_path)
  	{
  		my $msg = &xmlparser($xmlpath);
		&log("\n$msg") if($msg);
		exit if($msg);
		printf "xml files at $xmlpath format OK\n" if(!$msg);
	}
  }
  &buildProject($proj) if !$BUILD_BATCH{failed};
  $PUBLISHBUILD = 0 	if $proj->{skippublish};
  &publishBuild() if $PUBLISHBUILD and !$BUILD_BATCH{failed};
  $PUBLISHBUILD = 1; 
}

#&publishBuild() if $PUBLISHBUILD and !$BUILD_BATCH{failed};

&commitBuild2Source() if $COMMITBUILD2SOURCE and !$BUILD_BATCH{failed};

&emailReport() if $EMAILNOTIF;

&log("\n********** AutoBuild ends **********\n");

exit 0;

# ------------------------------------------------------
# usage screen
# ------------------------------------------------------
sub usage
{
   print <<EOF;
ZQAutoBuild, by Hui Shao <hui.shao\@i-zq.com>

Usage: ZQAutoBuild <options>
ZQAutoBuild is a simple Perl program that can build ZQ projects based on the project profiles

options
    --nightly
        turn on all the options designed for a nightly build
    --buildroot=<path>
        specify a location where this build will be work on
    --vodsdk=<path>
        specify the location of SeaChange VOD SDK
        
    --releaseroot=<path>
        specify the location where to put the output build(s)
				
    --projects="<PN>[,<PN>...]"
        specify the project(s) to build in this run, valid only in manual mode
        where <PN>s are the short name defined in the project build profiles
		
    --vssroot=<path>
        specify the VSS database
    --vssuser=<user>
        specify the login user name to the VSS database
    --vsspasswd=<password>
        password for the VSS user to access the database

    --skipgetsource
        skip to get source from the VSS
    --emailreport
        report the build result to people via email
    --emailto=<emails>
        must be specified if want to enable email		   
    --logfile=<filename>
        specify the build log file 
    --label=<label num>	        
	specify the source label number which you want to get
    --list
        list all the project profiles
    --test
        test build, will skip commit build to VSS and skip publishing
    --help
        display this screen

EOF

exit 0;
}

# ------------------------------------------------------
#  Get  project dependence based its profile
# paramters: $project->{dependence}
# ------------------------------------------------------

sub getdependence()
{
	my @a = ();
	my @btm = @PROJLIST;
	my $btm=scalar(@btm);
	my @tmp;
	for($itm=0; $itm<$btm; $itm++)
	{
		my @b=();
		push (@b,$btm[$itm]);
		push (@tmp,$btm[$itm]);
		my @atemm=();
		
	while(@tmp)
	{
		@tmp=();
		$countb=scalar(@b);
		my $bb;
		
		foreach $bb (@b)
		{
			print "bbbb=$bb\n";
			my $tempb = $bb->{dependence};
			if($tempb)
			{
				foreach $dependence (@$tempb)
				{
					print "dependence=$dependence\n";
				        die ("********Error:Project $bb\'s dependence include itself********") if ($dependence eq $bb);
					if(@atemm)
					{
					foreach $query1 (@atemm)
					{
						die ("********Error:Project $bb->{name} make dependence error********") if ($dependence eq $query1->{name});	
					}
				        }
				        my $ibb=0;
				        while($ibb<$countb)
				        {
				        	print "bingg $b[$ibb] have already exit\n" if($dependence eq $b[$ibb]);
				        	delete $b[$ibb] if($dependence eq $b[$ibb]);
				        	$ibb++;
				        }
					if(@tmp)
					{
						$counttmp=scalar(@tmp);
						$itmp=0;
						while($itmp<$counttmp)
						{
							print "$tmp[$itmp] have already existed at tmp\n" if ($dependence eq $tmp[$itmp]);
							delete $tmp[$itmp] if ($dependence eq $tmp[$itmp]);
						        $itmp++;
					        }
					}
					push (@tmp, $dependence);
				}
			}
		}
		$counta = scalar(@atemm);
	        splice (@atemm,$counta,0,@b);
		@b = @tmp;
	}
	foreach $ate (@atemm)
	{
		$aa=scalar(@a);
		for($btt=0; $btt<$aa; $btt++)
		{
		        print "$a[$btt] have already exist at forward project\n" if ($ate eq $a[$btt]);
			delete $a[$btt] if ($ate eq $a[$btt]);
		}
	}
	$fin=scalar(@a);
	splice (@a,$fin,0,@atemm);
	@PROJLIST1 = @a;
	}
		
	@PROJLIST = ();
	foreach $projectlist (@PROJLIST1)
	{
		if ($projectlist->{name})
		{
			push (@PROJLIST, $projectlist);
#			print "$projectlist->{name}\n";
                }
	}
        print "********Reverse the projectlist********\n";
        @PROJLIST = reverse @PROJLIST;
        print "projectlist=@PROJLIST\n";
        &log("\nBuilding project: @PROJLIST");
}

# ------------------------------------------------------
# build a project based its profile
# paramters: profile
# ------------------------------------------------------
sub buildProject
{
    my $profile = shift;
    return if !$profile;
#    $profile->{build_num} = $BUILD_BATCH{batch_num};
	
    &log("\nBuilding project: $profile->{name}");
    &log("$profile->{desc}") if $profile->{desc};
    &log("\tproduct version: $profile->{prod_ver}\n\tbuild: $profile->{build_num}");

    $BUILD_BATCH{crnt_step} = "building project: $profile->{name}";
    $BUILD_BATCH{comp_owner} =  $profile->{owner} if $profile->{owner};
    $BUILD_BATCH{comp_owner} = $EMAILATTNS if !$profile->{owner};
	
    if ($profile->{work_path})
    {
        &log("\tchange working directory to $BUILD_ROOT\\$profile->{work_path}");
        chdir(fixPathToOS($BUILD_ROOT."\\".$profile->{work_path})) or Die("failed to change diretory to $BUILD_ROOT\\$profile->{work_path}");
    }
	
    &log("\tclean up previous outputs");
	if ($profile->{output_pkg})
	{
		my $outputfiles = $profile->{output_pkg};
		my $ofile;
		foreach $ofile (@$outputfiles)
		{
			unlink(fixPathToOS("$BUILD_ROOT\\$ofile"));
		}
	}

    $BuildVerNum = $profile->{prod_ver}.".$profile->{build_num}";
    updateVersionH($profile);
    updateVersionI($profile);
    
	
#	updateVersionRC($profile);
    
    if ($profile->{build_cmds})
    {
	&log("**\tstart building");
	my $cmd;
        my $build_cmds = $profile->{build_cmds};
	
	if (!(NULL == %{$profile ->{environment}}))
	{
	&log("****project $profile->{name} temporary environment variable****");
	}
		

        foreach $cmd (@$build_cmds)
        {
           open ENV_FILE, ">env.bat";
        if (!(NULL == %{$profile ->{environment}}))
        {
        while (($key, $value) = each (%{$profile ->{environment}}))
	{
		printf ENV_FILE ("set $key=$value\n");
		&log("$key=$value");
	}
	}
           &log("\texec $cmd");
           printf ENV_FILE ("$cmd\n");
           
           close ENV_FILE;
	system("env.bat") ==0 or Die("Failed at command\n\t$cmd");
	unlink ("env.bat");
        }
    }
    else
    {
        &log("WARNNING\tno build procedure defined");
    }
	
	if ($profile->{output_pkg})
	{
		my $outputfiles = $profile->{output_pkg};
		my $ofile;
		foreach $ofile (@$outputfiles)
		{
			$ofile =fixPathToLogic($ofile);
			my @fns = split (/\//, $ofile);
			my $newfilename = $fns[scalar(@fns) -1];
			if ($newfilename eq "CMEV2Symbols.zip")
			{
			$newfilename = CMEV2Symbols.".V".$profile->{prod_ver}.".".$profile->{build_num}.".".zip;
			}
			else
			{
			@fns = split(/\./, $newfilename);
			$newfilename = $profile->{name}.".V".$profile->{prod_ver}.".".$profile->{build_num}.".".$fns[scalar(@fns) -1];
			}
			# TODO: check file existence
			push @OUTPUT_STACK, $ofile;
			push @OUTPUT_STACK, $newfilename;
		}
	}
    
	$BUILD_BATCH{crnt_step}="finished building project: $profile->{name}";
	$BUILD_BATCH{comp_owner} = $EMAILATTNS;
}

# ------------------------------------------------------
# build up the source tree for all the projects about to build
# ------------------------------------------------------
sub getSource
{
    # build up the list of source branch about to get from the VSS
    &log("\nPrepare source tree for building");
    my $proj;
    my $branch;
    my $wkdir = $BUILD_ROOT;
    &log("**\tcleanup the previous build tree $wkdir first");
    system("rd /s/q ".$wkdir);

    buildBranchStack() if !@SRC_BRANCH_STACK;
	
	$BUILD_BATCH{crnt_step}="get source from VSS";
    # get each necessary VSS branch for building
    loc_getsrc: foreach $branch (@SRC_BRANCH_STACK)
    {
        next loc_getsrc if !$branch;
		$branch = fixPathToLogic($branch);
        $wkdir = fixPathToOS($BUILD_ROOT.'/'.$branch);
        system("mkdir ".$wkdir);
        chdir($wkdir) or Die("ERROR\tfailed to change to directory $wkdir");
        
		&log("**\t\tget source: $branch");
        # call vss client
        my $cmd;
        $cmd = sprintf "echo Y | $SSCMD -command GetProject -server $VSSSERVER -name $VSSUSER -password $VSSPASSWD -database $SOSDATABASE -project $branch -workdir $wkdir -recursive -verbose" if(!trim($labelinfo));
        $cmd = sprintf "echo Y | $SSCMD -command GetProject -server $VSSSERVER -name $VSSUSER -password $VSSPASSWD -database $SOSDATABASE -project $branch -workdir $wkdir -recursive -verbose -label \"$labelinfo\"" if(trim($labelinfo));
        #my $cmd = sprintf "echo Y | $SSCMD GET \"-Y%s,%s\" \$/$branch/* -R -GWR -i-", $VSSUSER, $VSSPASSWD;
        system($cmd) == 0 or Die("ERROR\tCommand \'$cmd\' failed.");
    }

	&log('Source tree is ready');
	exit 0 if(trim($labelinfo));
	if (trim($labelinfo))
	{
		&renewBuildNumber();
		foreach $proj (@PROJLIST)
		{
			$BuildVerNum = $proj->{prod_ver}.".$proj->{build_num}";
   			updateVersionH($proj);
   			updateVersionI($proj);
		}
		exit 0;
	}
#	exit;
}

# ------------------------------------------------------
# build up the list of source branch about to get from the VSS
# ------------------------------------------------------
sub buildBranchStack
{
    my $proj;
    my $branch;

    &log("**\tprepare necessary branch list");
	
	my $buildsetting = fixPathToOS($BUILD_ROOT."/".$BUILD_SETTINGS);
	
    #push @SRC_BRANCH_STACK, $BUILD_SETTINGS if not -d $buildsetting;
    push @SRC_BRANCH_STACK, $BUILD_SETTINGS if(!trim($labelinfo));;
    foreach $proj (@PROJLIST)
    {
		$BUILD_BATCH{crnt_step} = "push source branch for project: ".$proj->{name};
        my $src_deps = $proj->{sources};
        my $dep;
        foreach $dep (@$src_deps)
        {
            my $need_push = 1;
            my $count = scalar(@SRC_BRANCH_STACK);
            my $i = 0;
            for($i=0 ; $i < $count ; $i ++)
            {
                my $len = length($dep);
                if (length($SRC_BRANCH_STACK[$i]) >= $len)
                {
                    my $tmpstr = substr($SRC_BRANCH_STACK[$i], 0, $len);
                    if ($tmpstr eq $dep)
                    {
                        $SRC_BRANCH_STACK[$i] = $dep if $need_push;
                        $SRC_BRANCH_STACK[$i] = '' if !$need_push;  # TODO: should delete this item from the stack, leave for now
                        $need_push = 0;
                    }
                }
            }
            push @SRC_BRANCH_STACK, $dep if $need_push;
        }
    }
}

# ------------------------------------------------------
# renew the build numbers based on the last successful build
# ------------------------------------------------------
sub renewBuildNumber
{
    &log("\nRenew batch number based on the last successful build");
	$BUILD_BATCH{crnt_step}='renew batch number based on the last successful build';
	my $batchfile = fixPathToOS($BUILD_ROOT."/".$BUILD_SETTINGS."/".$BUILD_BATCH{batch_fn});
	
    open BUILDBTH_F, "< $batchfile" or Die("failed to read $batchfile");
	my $line;
	$BUILD_BATCH{batch_num} =0;
	ver_h_next_line: while ($line = <BUILDBTH_F>)
    {
	    $_=$line;
		if(/^[\s]*[\s]*LAST_BUILD_BATCH[\s]*=[\s]*(\d+).*?/)
		{
		    $BUILD_BATCH{batch_num} = $1;
			next ver_h_next_line;
		}
		
		if(/^[\s]*[\s]*LB_([^\s]*)[\s]*=[\s]*(\d+).*?/)
		{
		    %BUILD_BATCH->{'LB_'.$1} = $2;
		    next ver_h_next_line;
		}
	}
	close BUILDBTH_F;
	chdir ($BUILD_ROOT);
	
	$BUILD_PRJS_DESC = "Projects to build:\n";
	$BUILD_PRJS_DESC_FORLABEL = "Projects to build:";
	
    foreach $proj (@PROJLIST)
    {
		my $key='LB_'.trim($proj->{name}).'_V'.trim($proj->{prod_ver});
		if ($BUILD_BATCH{$key})
		{
			$proj->{build_num} = $BUILD_BATCH{$key} +1;
		}
		else
		{
			$proj->{build_num} = 1;
		}
		$BUILD_BATCH{ $key } = $proj->{build_num};
		$BUILD_PRJS_DESC .= "[$proj->{name}, V$proj->{prod_ver}, build $proj->{build_num}]\n\t$proj->{desc}\n";
		$BUILD_PRJS_DESC_FORLABEL .= "[$proj->{name}, V$proj->{prod_ver}, build $proj->{build_num}]$proj->{desc}";	
                
    }
	
    my $logmsg = sprintf "**\tlast build batch# %d, current build batch# ", $BUILD_BATCH{batch_num};
    $BUILD_BATCH{batch_num} +=1;
    $logmsg .= $BUILD_BATCH{batch_num}; 
    &log($logmsg);		
}

# ------------------------------------------------------
# lable source with this build number
# ------------------------------------------------------
sub commitBuild2Source
{
    &log("\nCommit build to the source tree");

    $BUILD_BATCH{crnt_step}="commit build to the source tree ";
    my $label = "build batch# $BUILD_BATCH{batch_num}";
    my $labelcomment = $BUILD_PRJS_DESC_FORLABEL;
    my $comment = "Auto build process, batch# $BUILD_BATCH{batch_num} @ $BUILD_TIME \n";
    $comment .= "Type: nightly \n"  if $NIGHTLY;
    $comment .= "Type: manually \n" if !$NIGHTLY;
	$comment .= $BUILD_PRJS_DESC;
	
	buildBranchStack() if !@SRC_BRANCH_STACK;
	
	my $cmd;
    my $branch;

    &log("**\tlable related source branchs");
    loc_lblsrc: foreach $branch (@SRC_BRANCH_STACK)
    {
        next loc_lblsrc if !$branch;
		$branch = fixPathToLogic($branch);
        #call vss client
        $cmd = sprintf "echo Y | $SSCMD -command AddLabel -server $VSSSERVER -name $VSSUSER -password $VSSPASSWD -database $SOSDATABASE -project $branch -label \"$label\" -log \"$labelcomment\"";
        system($cmd) == 0 or &log("WARNING\tLabel command \'$cmd\' failed.");
    }

    &log("**\tupdate the build batch record");
    chdir (fixPathToOS($BUILD_ROOT."/$BUILD_SETTINGS"));
    
    $cmd = sprintf "echo Y | $SSCMD -command CheckOutFile -server $VSSSERVER -name $VSSUSER -password $VSSPASSWD -database $SOSDATABASE -project $BUILD_SETTINGS -file $BUILD_BATCH{batch_fn}";
    system($cmd);
    open BUILDBTH_F, "> $BUILD_BATCH{batch_fn}" or Die("failed to write $BUILD_BATCH{batch_fn}");
	
	if (!$BUILD_BATCH{failed})
	{
#	$comment = ~ s/#/\n/g;
		print BUILDBTH_F "$comment\n\n";
		print BUILDBTH_F "LAST_BUILD_TIME  = $BUILD_TIME\n";
		print BUILDBTH_F "LAST_BUILD_BATCH = $BUILD_BATCH{batch_num}\n";
		my($key, $value);
		while (($key, $value) = each(%BUILD_BATCH))
		{
			$_ = $key;
			printf BUILDBTH_F "%s = %s\n", $key, $value if (/LB_[^\s]*?/);
		}
		
		close BUILDBTH_F;
		
		$cmd = sprintf "echo Y | $SSCMD -command CheckInFile -server $VSSSERVER -name $VSSUSER -password $VSSPASSWD -database $SOSDATABASE -project $BUILD_SETTINGS -file $BUILD_BATCH{batch_fn}";
		#$cmd = sprintf "echo Y | $SSCMD CHECKIN \"-Y%s,%s\" \$/$BUILD_SETTINGS/$BUILD_BATCH{batch_fn} -i-", $VSSUSER, $VSSPASSWD;
		system($cmd) == 0 or Die("ERROR\tCommand \'$cmd\' failed.");
	}
	
    &log("**\tupdate the batch number record");	
    $cmd = sprintf "echo Y | $SSCMD -command CheckOutFile -server $VSSSERVER -name $VSSUSER -password $VSSPASSWD -database $SOSDATABASE -project $BUILD_SETTINGS -file $BUILD_BATCH{batch_vn}";
    system($cmd);
    
    my $batchnumfile = fixPathToOS($BUILD_ROOT."/".$BUILD_SETTINGS."/".$BUILD_BATCH{batch_vn});
    open (BUILDBTH_V, ">>$batchnumfile") or Die("failed to write $BUILD_BATCH{batch_vn}");
    
    foreach $proj (@PROJLIST)
    {
          if (!$BUILD_BATCH{failed})
                {
                     printf BUILDBTH_V "BN_$proj->{name}_V$proj->{prod_ver}.$proj->{build_num}=$BUILD_BATCH{batch_num}\n";
                }
         
    }
    close BUILDBTH_V;
    $cmd = sprintf "echo Y | $SSCMD -command CheckInFile -server $VSSSERVER -name $VSSUSER -password $VSSPASSWD -database $SOSDATABASE -project $BUILD_SETTINGS -file $BUILD_BATCH{batch_vn}";
    system($cmd);
    
    $BUILD_BATCH{crnt_step}="commited build to the source tree ";
}

# ------------------------------------------------------
# updateVersionH
# update version info in the c/c++ header file based on project profile
# parameters: profile
# ------------------------------------------------------
sub updateVersionH
{
    my $profile = shift;
    return if !$profile or !$profile->{rc_h_files};
	&log("\tupdate c/c++ header files with version info:");
    my $ver_file;
    my $ver_files = $profile->{rc_h_files};
	my @vernums = split(/\./, $profile->{prod_ver}) if $profile->{prod_ver};
	
    foreach $ver_file (@$ver_files)
    {
		&log("\t\t$ver_file");
		my $fn = fixPathToOS($BUILD_ROOT."/".$ver_file);
		system("attrib -R -H -S $fn");
		open VHFILE_I, "< $fn" or Die("failed to read $fn");
		open VHFILE_O, "> $fn.tmp" or Die("failed to create$fn.tmp");
		my $line;
		ver_h_next_line: while ($line = <VHFILE_I>)
		{
			$_=$line;
			# update ZQ_PRODUCT_VER_MAJOR
			if($profile->{prod_ver} and (/^[\s]*#[\s]*define[\s]+ZQ_PRODUCT_VER_MAJOR[\s]+.*?/))
			{
				printf VHFILE_O "#define ZQ_PRODUCT_VER_MAJOR  \t\%d\t// updated by autobuild process @ $BUILD_TIME\n", $vernums[0];
				next ver_h_next_line;
			}
			
			# update ZQ_PRODUCT_VER_MINOR
			if($profile->{prod_ver} and (/^[\s]*#[\s]*define[\s]+ZQ_PRODUCT_VER_MINOR[\s]+.*?/))
			{
				printf VHFILE_O "#define ZQ_PRODUCT_VER_MINOR  \t\%d\t// updated by autobuild process @ $BUILD_TIME\n", $vernums[1];
				next ver_h_next_line;
			}
			
			# update ZQ_PRODUCT_VER_PATCH
			if($profile->{prod_ver} and (/^[\s]*#[\s]*define[\s]+ZQ_PRODUCT_VER_PATCH[\s]+.*?/))
			{
				printf VHFILE_O "#define ZQ_PRODUCT_VER_PATCH  \t\%d\t// updated by autobuild process @ $BUILD_TIME\n", $vernums[2];
				next ver_h_next_line;
			}

			# update ZQ_PRODUCT_VER_BUILD
			if($profile->{build_num} and (/^[\s]*#[\s]*define[\s]+ZQ_PRODUCT_VER_BUILD[\s]+.*?/))
			{
				printf VHFILE_O "#define ZQ_PRODUCT_VER_BUILD  \t$profile->{build_num}\t// updated by autobuild process @ $BUILD_TIME\n";
				printf VHFILE_O "#define ZQ_PRODUCT_BUILDTIME   \t\"$BUILD_TIME\"\t// updated by autobuild process @ $BUILD_TIME\n";
				next ver_h_next_line;
			}

			# update ZQ_PRODUCT_VER_STR1
			if ($profile->{build_num} and (/^[\s]*#[\s]*define[\s]+ZQ_PRODUCT_VER_STR1[\s]+.*?/))
			{
				printf VHFILE_O "#define ZQ_PRODUCT_VER_STR1   \t\"%d.%d.%d.%d\"\t// updated by autobuild process @ $BUILD_TIME\n", $vernums[0], $vernums[1], $vernums[2], $profile->{build_num};
				next ver_h_next_line;
			}

			# update ZQ_PRODUCT_VER_STR2
			if ($profile->{build_num} and (/^[\s]*#[\s]*define[\s]+ZQ_PRODUCT_VER_STR2[\s]+.*?/))
			{
				printf VHFILE_O "#define ZQ_PRODUCT_VER_STR2   \t\"%d,%d,%d,%d\"\t// updated by autobuild process @ $BUILD_TIME\n", $vernums[0], $vernums[1], $vernums[2], $profile->{build_num};
				next ver_h_next_line;
			}

			# update ZQ_PRODUCT_VER_STR3
			if ($profile->{build_num} and (/^[\s]*#[\s]*define[\s]+ZQ_PRODUCT_VER_STR3[\s]+.*?/))
			{
				printf VHFILE_O "#define ZQ_PRODUCT_VER_STR3   \t\"V%d.%d.%d (build %d)\"\t// updated by autobuild process @ $BUILD_TIME\n", $vernums[0], $vernums[1], $vernums[2], $profile->{build_num};
				next ver_h_next_line;
			}

			printf VHFILE_O $line;
		}
		
		close VHFILE_O;
		close VHFILE_I;
		copy($fn.".tmp", $fn) and unlink($fn.".tmp") or Die("failed to update $fn");
	}
}

# ------------------------------------------------------
# updateVersionI
# update ice version info in the c/c++ header file based on project profile
# parameters: profile
# ------------------------------------------------------


sub updateVersionI
{
    my $profile = shift;
    return if !$profile or !$profile->{rc_h_files};
	&log("\tupdate c/c++ header files with version info:");
    my $ver_file;
    my $ver_files = $profile->{rc_h_files};
	my @vernums = split(/\./, $profile->{prod_ver}) if $profile->{prod_ver};
	
    foreach $ver_file (@$ver_files)
    {
		&log("\t\t$ver_file");
		my $fn = fixPathToOS($BUILD_ROOT."/".$ver_file);
		system("attrib -R -H -S $fn");
		open VIFILE_I, "< $fn" or Die("failed to read $fn");
		open VIFILE_O, "> $fn.tmp" or Die("failed to create$fn.tmp");
		my $line;
		ver_h_next_line: while ($line = <VIFILE_I>)
		{
			$_=$line;
			# update ZQ_PRODUCT_VER_MAJOR
			#if($profile->{prod_ver} and (/^[\s]*#[\s]*define[\s]+ZQ_PRODUCT_VER_MAJOR[\s]+.*?/))
			if($profile->{prod_ver} and (/^[\s]*const[\s]+int[\s]+MajorVersion[\s]+=[\s]+.*?/))
			{
				printf VIFILE_O "const int MajorVersion = \%d\;\n", $vernums[0];
				next ver_h_next_line;
			}
			
			# update ZQ_PRODUCT_VER_MINOR
			if($profile->{prod_ver} and (/^[\s]*const[\s]+int[\s]+MinorVersion[\s]+=[\s]+.*?/))
			{
				printf VIFILE_O "const int MinorVersion = \%d\;\n", $vernums[1];
				next ver_h_next_line;
			}
			
			# update ZQ_PRODUCT_VER_PATCH
			if($profile->{prod_ver} and (/^[\s]*const[\s]+int[\s]+PatchNumber[\s]+=[\s]+.*?/))
			{
				printf VIFILE_O "const int PatchNumber  = \%d\;\n", $vernums[2];
				next ver_h_next_line;
			}

			# update ZQ_PRODUCT_VER_BUILD
			if($profile->{build_num} and (/^[\s]*const[\s]+int[\s]+buildNumber[\s]+=[\s]+.*?/))
			{
				printf VIFILE_O "const int buildNumber  = $profile->{build_num};\n";
				next ver_h_next_line;
			}

			# update ZQ_PRODUCT_VER_STR1
			if ($profile->{build_num} and (/^[\s]*const[\s]+string[\s]+buildTime[\s]+=[\s]+.*?/))
			{
				printf VIFILE_O "const string buildTime = \"$BUILD_TIME\";\n";
				next ver_h_next_line;
			}

			printf VIFILE_O $line;
		}
		
		close VIFILE_O;
		close VIFILE_I;
		copy($fn.".tmp", $fn) and unlink($fn.".tmp") or Die("failed to update $fn");
	}
}

# ------------------------------------------------------
# trim a string
# ------------------------------------------------------
sub trim
{
my @out = @_;
   for (@out)
	{
   	s/^\s+//;
      s/\s+$//;
      s/^\n+$//;
   }
   return wantarray ? @out : $out[0];
}

# ------------------------------------------------------
# fixPathToOS
# ------------------------------------------------------
sub fixPathToOS
{
	my @out = @_;
	for (@out)
	{
		s/\//\\/g;
	   }
	return wantarray ? @out : $out[0];
}

# ------------------------------------------------------
# fixPathToLogic
# ------------------------------------------------------
sub fixPathToLogic
{
	my @out = @_;
	for (@out)
	{
		s/\\/\//g;
	}
	return wantarray ? @out : $out[0];
}

# ------------------------------------------------------
# publish buiilds to the release folder
# ------------------------------------------------------
sub publishBuild
{
	&log("\nPublish builds to the release folder $RELEASE_ROOT");
	Die("no build output defined") if !@OUTPUT_STACK;
	
	my $size = scalar(@OUTPUT_STACK);
	my $i;

	chdir(fixPathToOS($BUILD_ROOT));
	system("del /f/q *.*");
	
	my $files = "";

	for($i =1 ; $i < $size ; $i+=2)
	{
		&log("\tmove file $OUTPUT_STACK[$i] to $BUILD_ROOT/$OUTPUT_STACK[$i+1]");
		move(fixPathToOS($BUILD_ROOT."/".$OUTPUT_STACK[$i]), fixPathToOS($BUILD_ROOT."/".$OUTPUT_STACK[$i+1]))
			or &Die("ERROR: failed to move $OUTPUT_STACK[$i]");

		$files .= " ".$OUTPUT_STACK[$i+1];

		&log("\tpublish file $OUTPUT_STACK[$i] as $OUTPUT_STACK[$i+1]");
		copy(fixPathToOS("$BUILD_ROOT/".$OUTPUT_STACK[$i+1]), fixPathToOS($RELEASE_ROOT))
			or &Die("ERROR: failed to publish file $OUTPUT_STACK[$i]");
	}

	system("$MD5SUM $files > buildbatch_$BUILD_BATCH{batch_num}.md5");
	copy("buildbatch_$BUILD_BATCH{batch_num}.md5", fixPathToOS($RELEASE_ROOT))
}

# ------------------------------------------------------
# parse xml file format
# ------------------------------------------------------
sub xmlparser($)
{
	my $profile = shift;
	&log("\nparse xml format on directory $profile\n");
	my $xml_path = $BUILD_ROOT."\\".$profile;
	my $cmd = "$CMEPATH\\build\\utils\\xmlwf.exe  $xml_path\\*.xml >error";
	system($cmd);
	my $result = system ("$cmd");
	open CMDFILE, "<error";
	my $errmsg=<CMDFILE>;
	close CMDFILE;
	unlink error;
	return ($errmsg);
}

# ------------------------------------------------------
# build up email notification and send out
# ------------------------------------------------------
sub emailReport
{
    	&log("\nSend build execution report to $EMAILATTNS\n");
	# generate email body
	my $emailbody = fixPathToOS($BUILD_ROOT."/emailbody.txt");
	my $emailsubject;
	my $buildtype ="Manually";
	$buildtype ="Nightly"        if $NIGHTLY;
	my $attachment="";
	
	open(MAIL, "> $emailbody") or die "could not prepare emailbody";
	if (!$BUILD_BATCH{failed})
	{
		$emailsubject = "INFO: $buildtype build batch# $BUILD_BATCH{batch_num} completed";
		print MAIL "Dear team,\n\n";
		print MAIL "A new build has been completed successfully, attached please find the log messages about this build.\n";
		print MAIL "The following was the build summary:\n$BUILD_PRJS_DESC\n$BUILD_OPTS_DESC\n";
		print MAIL "\n\nThanks\n\nAutoBuild\nDepartment of Server Engineering One\nZQ Interactive.\n";
		print MAIL "\nPS, This email and the report were generated automatically, DO NOT reply\n";
	}
	else
	{
		$emailsubject = "ERROR: $buildtype build batch# $BUILD_BATCH{batch_num} failed";
		print MAIL "Dear component owner,\n\n";
		print MAIL "The build started at $BUILD_TIME was failed at the step of \n\t$BUILD_BATCH{crnt_step}\n\nAttached please find the log messages about this build.\n";
		print MAIL "The following was the build summary:\n$BUILD_PRJS_DESC\n$BUILD_OPTS_DESC\n";
		print MAIL "\nThe owner of the project where this step failed became the owner of the build process.\n";
		print MAIL "HE/SHE MUST CORRECT THE BUILD PROBLEM AS SOON AS POSSIBLE\n";
		print MAIL "\n\nThanks\n\nAutoBuild\nDepartment of Server Engineering One\nZQ Interactive.\n";
		print MAIL "\nPS, This email and the report were generated automatically, DO NOT reply\n";
	}
	
	close(MAIL);
	
	$attachment = " -attach \"$LOGFILE\"" if $LOGFILE;
	
	# send email out
	my $recipient = "-t \"$BUILD_BATCH{comp_owner}\" ";
	$recipient .= "-c \"$EMAILATTNS\" " if $EMAILATTNS ne $BUILD_BATCH{comp_owner};
	my $cmd = "$EMAILCMD \"$emailbody\" $recipient -s \"$emailsubject\" $attachment";
	system($cmd);
	
	chdir($BUILD_ROOT);
}

sub Die
{
   &log(@_);
   $BUILD_BATCH{failed} = 1;
   emailReport() if $EMAILNOTIF;
   exit -1;
}

sub log
{
   my ($log) = @_;
   # Only print to the log file if we are initialized.
   my (@lines) = split(/\n/,$log);
   my (@lt) = localtime;
   my $time = sprintf "%d\/%02d\/%02d %02d\:%02d\:%02d",
      $lt[5]+1900, $lt[4]+1, $lt[3], $lt[2], $lt[1], $lt[0];
   my $succ = open LOG, ">> $LOGFILE";
   foreach $line (@lines)
   {
      print LOG "$time  $line\n" if $succ;
      print "$line\n" if !$succ;
   }
   close LOG if $succ;
}

endprog:
__END__

:EndOfPerl