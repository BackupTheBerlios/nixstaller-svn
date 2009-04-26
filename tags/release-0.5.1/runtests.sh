#!/bin/sh

CFGFILE="tmp-testruns/config.lua"
RUNFILE="tmp-testruns/run.lua"
WELCOMEFILE="tmp-testruns/welcome"

createbase()
{
    mkdir tmp-testruns/
    
    mkdir tmp-testruns/files_all/
    
    cp example/vim/vim48x48.png tmp-testruns/
    
    cat << EOF > tmp-testruns/files_all/configure
#!/bin/sh
echo "Checking nothing ... yes"
EOF

}

rmbase()
{
    rm -rf tmp-testruns/
}

addlang()
{
    for L in $1; do
        cp -R lang/$L tmp-testruns/
    done
}

doinst()
{
    ./geninstall.sh tmp-testruns tests.sh && ./tests.sh
}

resetinst()
{
    rmbase
    createbase
}

rmbase
createbase

echo
echo "First Installer: Unknown lang, gzip compression, intropic"
echo

cat << EOF > $CFGFILE
cfg.appname = "test1"
cfg.intropic = "vim48x48.png"
cfg.archivetype = "gzip"
cfg.languages = { "english", "deutsch" }
EOF

echo "Howdy hoh" > $WELCOMEFILE

doinst

resetinst


echo
echo "Second Installer: Zero lang, bzip2 compression"
echo

cat << EOF > $CFGFILE
cfg.appname = "test2"
cfg.languages = { }
cfg.archivetype = "bzip2"
EOF

doinst

resetinst

echo
echo "Third Installer: Only default lang, lzma compression, shell commands"
echo

echo << EOF > $CFGFILE
cfg.appname = "test3"
EOF

cat << EOF > $RUNFILE
function Install()
    install.extractfiles()
    install.execute("./configure")
    
    for i=1, 25 do
        install.executeasroot("./configure")
    end
end
EOF

doinst

rmbase
