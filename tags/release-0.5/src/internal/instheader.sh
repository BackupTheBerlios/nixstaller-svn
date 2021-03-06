cat << EOF  > "$archname"
#!/bin/sh
# This script was generated using Nixstaller with Makeself $MS_VERSION
# The generated code is orginally based on 'makeself-header.sh' from Makeself,
# with modifications for Nixstaller.

CRCsum="$CRCsum"
MD5="$MD5sum"
TMPROOT=\${TMPDIR:=/tmp}

label="$LABEL"
script="$SCRIPT"
scriptargs="$SCRIPTARGS"
targetdir="$archdirname"
filesizes="$filesizes"
keep=$KEEP

[ "$INSTMODE" = "both" -o "$INSTMODE" = "attended" ] && CANATT=1 || CANATT=
[ "$INSTMODE" = "both" -o "$INSTMODE" = "unattended" ] && CANUNATT=1 || CANUNATT=

print_cmd_arg=""
if type printf > /dev/null; then
    print_cmd="printf"
    if [ \`uname\` != "OpenBSD" ]; then
        print_cmd_arg="--"
    fi
elif test -x /usr/ucb/echo; then
    print_cmd="/usr/ucb/echo"
else
    print_cmd="echo"
fi

unset CDPATH

MS_Printf()
{
    \$print_cmd \$print_cmd_arg "\$1"
}
    
MS_Progress()
{
    while read a; do
    MS_Printf .
    done
}

MS_dd()
{
    blocks=\`expr \$3 / 1024\`
    bytes=\`expr \$3 % 1024\`
    dd if="\$1" ibs=\$2 skip=1 obs=1024 conv=sync 2> /dev/null | \\
    { test \$blocks -gt 0 && dd ibs=1024 obs=1024 count=\$blocks ; \\
      test \$bytes  -gt 0 && dd ibs=1 obs=1024 count=\$bytes ; } 2> /dev/null
}

MS_Help()
{
    # Keep spacing between left and right column in sync with geninstall.lua!
    cat << EOH >&2
Usage: \$0 [options]"

[options] can be one of the following things (all are optional):

General:
--help, -h                  Print this message.
--info, -i                  Print some info about this installer.
--check, -c                 Check integrity of the archive.
EOH
    
    if [ ! -z "\$CANATT" ]; then
        echo '--frontend, -f <fr>         Specify frontend' >&2
    fi
    
    if [ ! -z "\$CANUNATT" ]; then
        echo '--unattended, -u            Run installer unattended (no interaction)' >&2
        if [ ! -z "$UNATTHELP" ]; then
            if [ "\$CANATT" ]; then
                MS_Printf "\nUnattended (requires -u/--unattended):\n"
            fi
            MS_Printf "$UNATTHELP"
        fi
    fi
    cat << EOH >&2

Advanced:
--keep                      Do not erase temporary target directory.
--nox11                     Do not spawn an xterm.
--target dir                Sets target directory for core contents (default is a temporary directory).
--tar arg1 [arg2 ...]       Access the core contents of the archive through the tar command.
EOH
}

# UNDONE: Check for other ways to find md5 checker
# UNDONE: Throw message incase no md5 checker was found.
MS_Check()
{
    OLD_PATH=\$PATH
    PATH=\${GUESS_MD5_PATH:-"\$OLD_PATH:/bin:/usr/bin:/sbin:/usr/local/ssl/bin:/usr/local/bin:/opt/openssl/bin"}
    
    # Added: Better way to get MD5 sum
    MD5CMD="\`which md5sum 2>/dev/null\`"
    if [ ! -f "\$MD5CMD" ]; then
        if [ -f "\`which md5 2>/dev/null\`" ]; then
            MD5CMD="\`which md5\`"
        elif [ -f "\`which digest 2>/dev/null\`" ]; then
            MD5CMD="\`which digest\` -a md5"
        fi
    fi
    
    PATH=\$OLD_PATH
    MS_Printf "Verifying archive integrity..."
    offset=\`head -n $SKIP "\$1" | wc -c | tr -d " "\`
    verb=\$2
    i=1
    for s in \$filesizes
    do
    crc=\`echo \$CRCsum | cut -d" " -f\$i\`
    if [ ! -z "\$MD5CMD" ]; then
        md5=\`echo \$MD5 | cut -d" " -f\$i\`
        if test \$md5 = "00000000000000000000000000000000"; then
        test x\$verb = xy && echo " \$1 does not contain an embedded MD5 checksum." >&2
        else
        md5sum=\`MS_dd "\$1" \$offset \$s | \$MD5CMD | cut -b-32\`;
        if test "\$md5sum" != "\$md5"; then
            echo "Error in MD5 checksums: \$md5sum is different from \$md5" >&2
            exit 2
        else
            test x\$verb = xy && MS_Printf " MD5 checksums are OK." >&2
        fi
        crc="0000000000"; verb=n
        fi
    fi
    if test \$crc = "0000000000"; then
        test x\$verb = xy && echo " \$1 does not contain a CRC checksum." >&2
    else
        sum1=\`MS_dd "\$1" \$offset \$s | CMD_ENV=xpg4 cksum | awk '{print \$1}'\`
        if test "\$sum1" = "\$crc"; then
        test x\$verb = xy && MS_Printf " CRC checksums are OK." >&2
        else
        echo "Error in checksums: \$sum1 is different from \$crc"
        exit 2;
        fi
    fi
    i=\`expr \$i + 1\`
    offset=\`expr \$offset + \$s\`
    done
    echo " All good."
}

UnTAR()
{
    tar \$1vf - 2>&1 || { echo Extraction failed. > /dev/tty; kill -15 \$$; }
}

PrintInfo()
{
    if [ ! -z "\${2}" ]; then
        echo "\$1" "\$2"
    fi
}

finish=true
xterm_loop=
nox11=$NOX11
copy=$COPY
ownership=y
verbose=n

initargs="\$@"

[ "$INSTMODE" = "unattended" ] && { unattended=1; scriptargs="\$scriptargs --unattended"; } || unattended=

# UNDONE: Remove undocumented options (?)
while true
do
    case "\$1" in
    -h | --help)
    MS_Help
    exit 0
    ;;
    -i | --info)
    PrintInfo "Date of packaging            " "$DATE"
    PrintInfo "Installer name               " "$INSTNAME"
    PrintInfo "Compression                  " "$INSTPACK"
    PrintInfo 'Mode (Attended/Unattended)   ' "$INSTMODE"
    PrintInfo "Supported OSs                " "$INSTOS"
    PrintInfo "Supported CPU Arch's         " "$INSTARCH"
    PrintInfo "Frontends                    " "$INSTFRONTENDS"
    PrintInfo "Package name                 " "$PKGNAME"
    PrintInfo "Package version              " "$PKGVERSION"
    PrintInfo "Package summary              " "$PKGSUMMARY"
    PrintInfo "Package description          " "$PKGDESC"
    PrintInfo "Package group                " "$PKGGROUP"
    PrintInfo "Package license              " "$PKGLICENSE"
    PrintInfo "Package maintainer           " "$PKGMAINT"
    PrintInfo "Package URL                  " "$PKGURL"
    exit 0
    ;;
    --dumpconf)
    echo LABEL=\"\$label\"
    echo SCRIPT=\"\$script\"
    echo SCRIPTARGS=\"\$scriptargs\"
    echo archdirname=\"$archdirname\"
    echo KEEP=$KEEP
    echo COMPRESS=$COMPRESS
    echo filesizes=\"\$filesizes\"
    echo CRCsum=\"\$CRCsum\"
    echo MD5sum=\"\$MD5\"
    echo OLDUSIZE=$USIZE
    echo OLDSKIP=`expr $SKIP + 1`
    exit 0
    ;;
    --lsm)
cat << EOLSM
EOF
eval "$LSM_CMD"
cat << EOF  >> "$archname"
EOLSM
    exit 0
    ;;
    --list)
    echo Target directory: \$targetdir
    offset=\`head -n $SKIP "\$0" | wc -c | tr -d " "\`
    for s in \$filesizes
    do
        MS_dd "\$0" \$offset \$s | eval "$GUNZIP_CMD" | UnTAR t
        offset=\`expr \$offset + \$s\`
    done
    exit 0
    ;;
    --tar)
    offset=\`head -n $SKIP "\$0" | wc -c | tr -d " "\`
    arg1="\$2"
    shift 2
    for s in \$filesizes
    do
        MS_dd "\$0" \$offset \$s | eval "$GUNZIP_CMD" | tar "\$arg1" - \$*
        offset=\`expr \$offset + \$s\`
    done
    exit 0
    ;;
    -c | --check)
    MS_Check "\$0" y
    exit 0
    ;;
    --confirm)
    verbose=y
    shift
    ;;
    --noexec)
    script=""
    shift
    ;;
    --keep)
    keep=y
    shift
    ;;
    --target)
    keep=y
    targetdir=\${2:-.}
    shift 2
    ;;
    --nox11)
    nox11=y
    shift
    ;;
    --nochown)
    ownership=n
    shift
    ;;
    --xwin)
    finish="echo Press Return to close this window...; read junk"
    xterm_loop=1
    shift
    ;;
    -f | --frontend)
    if [ ! -z "\$CANATT" ]; then
        scriptargs="\$scriptargs --frontend \$2"
        shift 2
    else
        echo "Attended installations not enabled."
        MS_Help
        exit 1
    fi
    ;;
    -u | --unattended)
    if [ ! -z "\$CANUNATT" ]; then
        scriptargs="\$scriptargs --unattended"
        unattended=1
        shift
    else
        echo "Unattended installations not enabled."
        MS_Help
        exit 1
    fi
    ;;
    # Auto generated options
$OPTHANDLERS
    --phase2)
    copy=phase2
    shift
    ;;
    --)
    shift
    break ;;
    -*)
    echo Unrecognized flag : "\$1" >&2
    MS_Help
    exit 1
    ;;
    *)
    break ;;
    esac
done

$OPTCHECKERS

if [ ! -z "\$gaveunopt" -a -z "\$unattended" ]; then
    echo "Option(s) for unattended installation given while launching a regular installation. Specify --unattended or -u to start an unattended installation."
    exit 1
fi

case "\$copy" in
copy)
    tmpdir=\$TMPROOT/makeself.\$RANDOM.\`date +"%y%m%d%H%M%S"\`.\$\$
    mkdir "\$tmpdir" || {
    echo "Could not create temporary directory \$tmpdir" >&2
    exit 1
    }
    SCRIPT_COPY="\$tmpdir/makeself"
    echo "Copying to a temporary location..." >&2
    cp "\$0" "\$SCRIPT_COPY"
    chmod +x "\$SCRIPT_COPY"
    cd "\$TMPROOT"
    exec "\$SCRIPT_COPY" --phase2
    ;;
phase2)
    finish="\$finish ; rm -rf \`dirname \$0\`"
    ;;
esac

if test "\$nox11" = "n"; then
    if tty -s; then                 # Do we have a terminal?
    :
    else
        if test x"\$DISPLAY" != x -a x"\$xterm_loop" = x; then  # No, but do we have X?
            if xset q > /dev/null 2>&1; then # Check for valid DISPLAY variable
                GUESS_XTERMS="xterm rxvt dtterm eterm Eterm kvt konsole aterm"
                for a in \$GUESS_XTERMS; do
                    if type \$a >/dev/null 2>&1; then
                        XTERM=\$a
                        break
                    fi
                done
                chmod a+x \$0 || echo Please add execution rights on \$0
                if test \`echo "\$0" | cut -c1\` = "/"; then # Spawn a terminal!
                    exec \$XTERM -title "\$label" -e "\$0" --xwin "\$initargs"
                else
                    exec \$XTERM -title "\$label" -e "./\$0" --xwin "\$initargs"
                fi
            fi
        fi
    fi
fi

if test "\$targetdir" = "."; then
    tmpdir="."
else
    if test "\$keep" = y; then
    echo "Creating directory \$targetdir" >&2
    tmpdir="\$targetdir"
    dashp="-p"
    else
    tmpdir="\$TMPROOT/selfgz\$\$\$RANDOM"
    dashp=""
    fi
    mkdir \$dashp \$tmpdir || {
    echo 'Cannot create target directory' \$tmpdir >&2
    echo 'You should try option --target OtherDirectory' >&2
    eval \$finish
    exit 1
    }
fi

location="\`pwd\`"
if test x\$SETUP_NOCHECK != x1; then
    MS_Check "\$0"
fi
offset=\`head -n $SKIP "\$0" | wc -c | tr -d " "\`

if test x"\$verbose" = xy; then
    MS_Printf "About to extract $USIZE KB in \$tmpdir ... Proceed ? [Y/n] "
    read yn
    if test x"\$yn" = xn; then
        eval \$finish; exit 1
    fi
fi

MS_Printf "Uncompressing \$label"
res=3
if test "\$keep" = n; then
    trap 'echo Signal caught, cleaning up >&2; cd \$TMPROOT; /bin/rm -rf \$tmpdir; eval \$finish; exit 15' 1 2 3 15
fi

for s in \$filesizes
do
    if MS_dd "\$0" \$offset \$s | eval "$GUNZIP_CMD" | ( cd "\$tmpdir"; UnTAR x ) | MS_Progress; then
        if test x"\$ownership" = xy; then
            (PATH=/usr/xpg4/bin:\$PATH; cd "\$tmpdir"; chown -R \`id -u\` .;  chgrp -R \`id -g\` .)
        fi
    else
        echo
        echo "Unable to decompress \$0" >&2
        eval \$finish; exit 1
    fi
    offset=\`expr \$offset + \$s\`
done
echo

cd "\$tmpdir"
res=0
if test x"\$script" != x; then
    if test x"\$verbose" = xy; then
        MS_Printf "OK to execute: \$script \$scriptargs \$* ? [Y/n] "
        read yn
        if test x"\$yn" = x -o x"\$yn" = xy -o x"\$yn" = xY; then
            eval \$script \$scriptargs \$*; res=\$?;
        fi
    else
        eval \$script \$scriptargs \$*; res=\$?
    fi
    if test \$res -ne 0; then
        test x"\$verbose" = xy && echo "The program '\$script' returned an error code (\$res)" >&2
    fi
fi
if test "\$keep" = n; then
    cd \$TMPROOT
    /bin/rm -rf \$tmpdir
fi
eval \$finish; exit \$res
EOF
