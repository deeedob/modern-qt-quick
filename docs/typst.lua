-- PDF-only adjustments. Code annotations print as "Line 5" in the PDF, so annotated code
-- blocks get line numbers there; `.more` lines get the same tinted box as on the website.

if not quarto.doc.is_format("typst") then
  return {}
end

local function isAnnotated(code)
  for line in (code.text .. "\n"):gmatch("(.-)\n") do
    if line:find("<%d+>%s*$") then
      return true
    end
  end
  return false
end

return {
  {
    Meta = function(meta)
      local ok, sha = pcall(pandoc.pipe, "git", { "rev-parse", "--short=12", "HEAD" }, "")
      if ok then
        meta["build-sha"] = pandoc.MetaString(sha:gsub("%s+$", ""))
      end
      local tagged, tag = pcall(pandoc.pipe, "git", { "tag", "--points-at", "HEAD" }, "")
      tag = tagged and tag:match("[^%s]+") or nil
      if tag then
        meta["build-tag"] = pandoc.MetaString(tag)
      end
      return meta
    end,
    CodeBlock = function(code)
      if isAnnotated(code) and not code.classes:includes("number-lines") then
        code.classes:insert("number-lines")
        return code
      end
    end,
    Div = function(div)
      if div.classes:includes("source-snippet") then
        local code = div.content[2]
        if code and code.t == "CodeBlock" then
          local _, lines = code.text:gsub("\n", "")
          if lines < 15 then
            div.content:insert(1, pandoc.RawBlock("typst", "#block(breakable: false)["))
            div.content:insert(pandoc.RawBlock("typst", "]"))
            return div
          end
        end
      end
      if div.classes:includes("more") then
        div.content:insert(1, pandoc.RawBlock("typst",
          '#block(width: 100%, inset: (x: 10pt, y: 7pt), radius: 3pt, fill: rgb("#5b5bd6").lighten(91%), stroke: (left: 2pt + rgb("#5b5bd6")))['))
        div.content:insert(pandoc.RawBlock("typst", "]"))
        return div
      end
    end,
  },
}
