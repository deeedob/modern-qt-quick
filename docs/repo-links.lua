-- Turns repository paths in `.more` lines, such as `src/app/main.cpp`, into links to that
-- file on GitHub. Only paths that exist in the repository become links.

local repoUrl
local branch = "main"
local repoRoot = pandoc.path.join({ quarto.project.directory, ".." })

local function isFile(path)
  local file = io.open(path, "r")
  if file then
    file:close()
  end
  return file ~= nil
end

local function isDirectory(path)
  return pcall(pandoc.system.list_directory, path)
end

local function linkFor(text)
  if text:find("%s") or text:find("^%.%.") then
    return nil
  end
  local path = text:gsub("/$", "")
  local full = pandoc.path.join({ repoRoot, path })
  -- Directories first: io.open succeeds on a directory on macOS.
  if isDirectory(full) then
    return repoUrl .. "/tree/" .. branch .. "/" .. path
  end
  if isFile(full) then
    return repoUrl .. "/blob/" .. branch .. "/" .. path
  end
  return nil
end

local function linkCode(code)
  local target = linkFor(code.text)
  if target then
    return pandoc.Link({ code }, target)
  end
end

return {
  {
    Meta = function(meta)
      local book = meta.book
      if book and book["repo-url"] then
        repoUrl = pandoc.utils.stringify(book["repo-url"]):gsub("/$", "")
      end
      if book and book["repo-branch"] then
        branch = pandoc.utils.stringify(book["repo-branch"])
      end
    end,
  },
  {
    Div = function(div)
      if repoUrl and div.classes:includes("more") then
        return div:walk({ Code = linkCode })
      end
    end,
  },
}
