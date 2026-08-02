pio run -e robot -t upload
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

pio run -e display -t upload
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

pio run -e remote -t upload
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }