-- The two lines in a .snippet code block select a literal source range.

local repoUrl
local branch = "main"
local repoRoot = pandoc.path.join({ quarto.project.directory, ".." })

local function fail(message)
  io.stderr:write("source snippet: " .. message .. "\n")
  os.exit(1)
end

local function count(text, needle, from)
  local found = 0
  local position = from or 1
  while true do
    local startAt = text:find(needle, position, true)
    if not startAt then
      return found
    end
    found = found + 1
    position = startAt + #needle
  end
end

local function lineAt(text, position)
  local _, lines = text:sub(1, position):gsub("\n", "")
  return lines + 1
end

local function readMeta(meta)
  local book = meta.book
  if book and book["repo-url"] then
    repoUrl = pandoc.utils.stringify(book["repo-url"]):gsub("/$", "")
  end
  if book and book["repo-branch"] then
    branch = pandoc.utils.stringify(book["repo-branch"])
  end
end

local function sourceSnippet(div)
  if not div.classes:includes("snippet") then
    return nil
  end

  local source = div.attributes.source
  if not source then return fail(".snippet needs a source attribute") end
  if source:find("^%.%.") or source:find("%s") then
    return fail("invalid source path " .. source)
  end

  local handle = io.open(pandoc.path.join({ repoRoot, source }), "r")
  if not handle then
    return fail("cannot read " .. source)
  end
  local content = handle:read("*a")
  handle:close()

  if #div.content ~= 1 or div.content[1].t ~= "CodeBlock" then
    return fail(".snippet must contain one code block")
  end
  local block = div.content[1]
  local bounds = {}
  for line in (block.text .. "\n"):gmatch("(.-)\n") do
    if line ~= "" then bounds[#bounds + 1] = line:gsub("\\n", "\n") end
  end
  if #bounds ~= 2 then
    return fail("provide one start and one stop text in " .. source)
  end
  local pieces, links = {}, {}
  local previousEnd = 0
  for i = 1, #bounds, 2 do
    local startText, endText = bounds[i], bounds[i + 1]
    if count(content, startText) ~= 1 then
      return fail("start text must occur exactly once in " .. source .. ": " .. startText)
    end
    local startAt = content:find(startText, 1, true)
    if startAt <= previousEnd or count(content, endText, startAt) ~= 1 then
      return fail("stop text must occur exactly once after start, with ranges in order: " .. source .. ": " .. endText)
    end
    local endAt = content:find(endText, startAt, true) + #endText - 1
    previousEnd = endAt
    local firstLine, lastLine = lineAt(content, startAt), lineAt(content, endAt)
    pieces[#pieces + 1] = content:sub(startAt, endAt)
    if #links > 0 then links[#links + 1] = pandoc.Str(", "); end
    local label = (#links == 0 and source .. ":" or "") .. firstLine .. "-" .. lastLine
    local url = repoUrl .. "/blob/" .. branch .. "/" .. source .. "#L" .. firstLine .. "-L" .. lastLine
    links[#links + 1] = pandoc.Link({ pandoc.Code(label) }, url, "Source excerpt")
  end
  block.text = pieces[1]
  local caption = pandoc.Para(links)
  return pandoc.Div({ caption, block }, pandoc.Attr("", { "source-snippet" }))
end

return {
  { Meta = readMeta },
  { Div = sourceSnippet },
}
