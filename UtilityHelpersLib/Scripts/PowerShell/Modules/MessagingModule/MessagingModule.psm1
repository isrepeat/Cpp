function NewLine {
    Write-Host ""
}


function Message {
    param(
        [Parameter(Position = 0, Mandatory = $true)]
        [string]$text,
        [string]$color = "Gray",
        [string]$prefix = ""
    )
	
    Write-Host "$prefix$text" -ForegroundColor $color
}

function MessageError {
    param (
        [Parameter(Position = 0, Mandatory = $true)]
        [string]$text
    )
    Message -color Red -text $text
}

function MessageAction {
    param (
		[Parameter(Position = 0, Mandatory = $true)]
        [string]$text
    )
    Message -color Yellow -text $text
}


function TestColoredMessagePalette {
    $colors = @(
        "DarkBlue", "DarkGreen", "DarkCyan", "DarkRed", "DarkMagenta", "DarkYellow", "Gray",
        "DarkGray", "Blue", "Green", "Cyan", "Red", "Magenta", "Yellow", "White"
    )

    $message = "The quick brown fox jumps over the lazy dog"

    Message -color Cyan -text "`nColor Palette Test:"
    foreach ($color in $colors) {
        Message -color $color -text ("{0,-12} : {1}" -f $color, $message)
    }
}


function WrapInQuotesIfNeeded {
    param ([string]$text)

    if ($text.StartsWith('"') -and $text.EndsWith('"')) {
        return $text
    }

    return '"' + $text + '"'
}


Export-ModuleMember -Function @(
    'Message',
    'MessageError',
    'MessageAction',
    'NewLine',
    'TestColoredMessagePalette',
    'WrapInQuotesIfNeeded'
)