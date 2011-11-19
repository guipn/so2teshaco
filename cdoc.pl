# CDoc/gdjs
#
# This work is licensed under the Creative 
# Commons Attribution-ShareAlike 3.0 Unported License. 
# To view a copy of this license, visit 
# http://creativecommons.org/licenses/by-sa/3.0/
# or send a letter to Creative Commons, 444 Castro Street, 
# Suite 900, Mountain View, California, 94041, USA.

use strict;
use warnings;
use Cwd;
use CDocTemplates;
use XML::Simple;

my (@dirs, @files, %whitelist);

my $xml           = XMLin();
my $cwd           = getcwd;
my $docroot       = $cwd . '/' . $xml->{docroot};
my $authorshipurl = $xml->{authorshipurl};
my $pname         = $xml->{name};
my $pversion      = $xml->{version};
my $purl          = $xml->{projecturl};
my $pauthorship   = $xml->{authorship};
my $pintro        = $xml->{introduction};

if (-e $docroot)
{
    print "\n\t$docroot already exists... using it.";
}
else
{
    print "Creating documentation directory $docroot.";
    mkdir $docroot;
}

push @dirs, $cwd;

$whitelist{$_} = 1 for @{ $xml->{process} };

for (@dirs)
{
    next unless $_ ne $docroot;

    opendir my $dir, $_ or die "Failed to open directory $_.";

    while (defined (my $file = readdir $dir))
    {
	next unless $file ne '.' and $file ne '..' 
				 and $file ne $0
				 and whitelisted($file);

	-f "$_/$file" and push @files, "$_/$file";
	-d $file      and push @dirs,  $file;
    }
}

@files == 0 and die 'Nothing to process.';

generate_index(@files);
process($_) for @files;

print "\n\n\t\tDone.\n\n";



 ## Helper subs ##


sub expand_basic_tags
{
    my ($path, $content) = @_ or die;

    my $filename = getfname($path);

    $content =~ s/<ProjectName>/$pname/g;
    $content =~ s/<FileName>/$filename/g;
    $content =~ s/<ProjectURL>/$purl/g;
    $content =~ s/<ProjectVersion>/$pversion/g;
    $content =~ s/<AuthorshipURL>/$authorshipurl/g;
    $content =~ s/<Authorship>/$pauthorship/g;

    $content;
}


sub expand_function_section
{
    my ($output, @functions) = @_ or die;

    if (@functions)
    {
	my $funsection =  getfunsection();
	$output        =~ s/<FunSection>/$funsection/;
    }
    else
    {
	$output =~ s/<FunSection>//;
    }

    $output;
}

# Gets the name of some file given its path

sub getfname
{
    (split /\//, shift)[-1];
}


sub generate_index
{
    print "\n\tComputing index...";
    my @filepaths = @_ or die;

    my $output = getindex();

    $output =~ s/<ProjectName>/$pname/g;
    $output =~ s/<ProjectURL>/$purl/g;
    $output =~ s/<ProjectVersion>/$pversion/g;
    $output =~ s/<AuthorshipURL>/$authorshipurl/g;
    $output =~ s/<Authorship>/$pauthorship/g;
    $output =~ s/<Introduction>/$pintro/g;
    $output =  insert_index_lines($output, @filepaths);

    writefile("$docroot/index.htm", $output);
    writecss();
}


sub insert_defines
{
    my ($output, @defines) = @_ or die;

    print "\n\t\tComputing #defines...";

    if (@defines)
    {
	my $defsection = getdefsection();
	$output        =~ s/<DefSection>/$defsection/;
    }
    else
    {
	$output =~ s/<DefSection>//;
	return $output;
    }

    for (@defines)
    {
	my $line = getdefineline();
	$line    =~ s/<Macro>/$_/;
	$output  =~ s/<Definitions>/$line<Definitions>/;
    }

    $output =~ s/<Definitions>//;
    $output;
}


# Comments starting with /** and containing a 
# $$ anywhere will be considered the introductory 
# text for the given source file. Convention should
# dictate that such introductions be placed at the
# very top of the source file being introducted. Below
# is an example of the usage of this feature:
#
# (main.c)
# /**
#  * $$
#  *
#  * This is the entry point for this program.
#  * If you have any doubts or suggestions, please
#  * contact me at foo@bar.com.
#  *
#  */
# ... (rest of the code)
#
# Newlines are automatically substituted by <br/>.

sub insert_file_intro
{
    my ($output, $comment) = @_ or die;

    print "\n\t\tComputing introductory section...";

    $comment =~ s/\$\$//;
    $comment =~ s/\s\*\s//g;
    $comment =~ s/\s*\/\*\*\s*//g;
    $comment =~ s/\s*\*\/\s*//g;
    $comment =~ s/\n/<br\/>/g;

    my $introduction = getfileintroduction();

    $introduction =~ s/<FIntroText>/$comment/;
    $output       =~ s/<FileIntroduction>/$introduction/;
    $output;
}


sub insert_includes
{
    my ($output, @includes) = @_ or die;

    print "\n\t\tComputing #includes...";

    if (@includes)
    {
	my $incsection = getincsection();
	$output        =~ s/<IncSection>/$incsection/;
    }
    else
    {
	$output =~ s/<IncSection>//;
	return $output;
    }

    for (@includes)
    {
	my $line = getincludeline();
	if (my ($included) = (/include\s+["<](.+)[">]/))
	{
	    $line   =~ s/<IncludedFile>/$included/;
	    $output =~ s/<Inclusions>/$line    <Inclusions>/;
	}
    }
    
    $output =~ s/<Inclusions>\n//;
    $output;
}


sub insert_index_lines
{
    my ($output, @filepaths) = @_ or die;

    for (@filepaths)
    {
	$_       =  getfname($_);
	my $line =  getindexline();
	$line    =~ s/<FileName>/$_/g;
	$output  =~ s/<IndexLines>/$line<IndexLines>/;
    }

    $output =~ s/<IndexLines>//;
    $output;
}


sub insert_prototypes
{
    my ($output, @functions) = @_ or die;

    print "\n\t\tComputing prototypes...";

    for (@functions)
    {
	my $proto = getprototype();
	
	defined $_->{name} and $proto =~ s/<FunctionName>/$_->{name}/g;
	defined $_->{prot} and $proto =~ s/<FunctionPrototype>/$_->{prot}/g;
	defined $_->{desc} and $proto =~ s/<FunctionDescription>/$_->{desc}/g;
	defined $_->{retd} and s/<ReturnsDescription>/$_->{retd}/;
	s/<ReturnsDescription>//;

	$output =~ s/<Prototypes>/$proto<Prototypes>/g;
    }

    $output =~ s/<Prototypes>//g;
    $output;
}


sub insert_summary_lines
{
    my ($output, @functions) = @_ or die;

    print "\n\t\tCreating summary...";

    for (@functions)
    {
	my $line = getsummaryline();
	
	defined $_->{name} and $line =~ s/<FunctionName>/$_->{name}/g;
	defined $_->{sdes} and $line =~ s/<ShortDescription>/$_->{sdes}/g;

	$output =~ s/<SummaryLines>/$line<SummaryLines>/g;
    }

    $output =~ s/<SummaryLines>//g;
    $output;
}


# A function is:
#
# (
#     name => 'funcname',
#     prot => 'prototype',
#     desc => 'description',
#     sdes => 'short description',
#     retd => 'return description'
# )

sub process
{
    my $filepath = shift or die;
    my $output   = expand_basic_tags($filepath, getsourcefile());
    my (@functions, @includes, @defines);

    open my $file, $filepath or die "Failed while trying to process $filepath.";
    local $/ = "\n\n";

    print "\n\tParsing $filepath...";
    
    while (<$file>)
    {
	m{(\#include\s.+)} and push @includes, $1;
	m{(\#define\s.+)}  and push @defines, $1;

	next unless m{\/\*\*};

	m{\$\$} and $output = insert_file_intro($output, $_) and next;

	s/\s\*\s//g;
	my ($name) = (m{\$name\s(.+)});
	s/\$name\s(.+)//g;
	my ($sdes) = (m{\$sdes\s([^;]+)}s);
	s/\$sdes\s//s;
	s/\.;/./;
	my ($proto) = (m{\$proto\s(.+)});
	s/\$proto\s(.+)//g;
	my ($returns) = (m{\$returns\s(.+)});
	s/\$returns\s(.+)//g;
	s/\$proto\s(.+)//g;
	s/\s*\/\*\*\s*//g;
	s/\s*\*\/\s*//g;

	push @functions, {
	    name => $name,
	    prot => $proto,
	    desc => $_,
	    sdes => $sdes,
	    retd => $returns
	};
    }

    close $file;
    
    $output =  expand_function_section($output, @functions);
    $output =  insert_summary_lines($output, @functions);
    $output =  insert_prototypes($output, @functions);
    $output =  insert_includes($output, @includes);
    $output =  insert_defines($output, @defines);
    $output =~ s/<FileIntroduction>//;

    writefile("$filepath.htm", $output);
}


sub whitelisted
{
    my $file = shift or die;

    my ($ext) = ($file =~ m{.*(\..+)});
    return defined( $ext ? $whitelist{$ext} : $whitelist{$file} );
}


sub writecss
{
    writefile("$docroot/style.css", getCSS());
}


sub writefile
{
    my ($path, $content) = @_ or die;

    print "\n\tWriting $path...";

    my $justname = getfname($path);

    open my $f, '>', "$docroot/$justname" or die;
    print $f $content;
    close $f;
}
