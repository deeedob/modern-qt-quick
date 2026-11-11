// A compact, one-sided book: no part pages and no blank pages. Replaces the orange-book
// layout that Quarto uses for Typst books by default.

#let guide-accent = rgb("#5b5bd6")
#let guide-pending-part = state("guide-pending-part", none)

#let part(title) = {
  [#metadata(title) <guide-part>]
  guide-pending-part.update(title)
}

#let guide-contents() = context {
  let entries = query(selector(<guide-part>).or(heading.where(level: 1)).or(heading.where(level: 2)))
  set par(justify: false)
  show link: set text(fill: black)
  for entry in entries {
    if entry.func() == metadata {
      block(above: 1.1em, below: 0.4em, text(size: 0.8em, weight: "bold", fill: guide-accent, tracking: 0.06em, upper(entry.value)))
      continue
    }
    if entry.level == 2 and entry.numbering == none { continue }
    let loc = entry.location()
    let number = if entry.numbering != none { numbering("1.1", ..counter(heading).at(loc)) }
    let page = counter(page).at(loc).first()
    let row = if entry.level == 1 {
      block(above: 0.5em, below: 0.25em, link(loc, grid(columns: (2.4em, 1fr, auto),
        strong(number), strong(entry.body), strong(str(page)))))
    } else {
      block(above: 0.2em, below: 0.2em, link(loc, grid(columns: (2.4em, 3em, 1fr, auto),
        [], text(fill: luma(90), number), [#entry.body #box(width: 1fr, repeat[.#h(0.4em)])], str(page))))
    }
    row
  }
}

#let book(title: none, subtitle: none, author: none, date: none, site: none, repo: none, build: none, font-size: 10.5pt, body) = {
  set document(title: title, author: if author != none { author } else { () })
  set text(size: font-size)
  set par(justify: true, leading: 0.62em, spacing: 1em)
  set page(
    numbering: "1",
    header: context {
      let here-page = here().page()
      if query(heading.where(level: 1)).any(h => h.location().page() == here-page) { return }
      let chapters = query(heading.where(level: 1).before(here()))
      if chapters.len() > 0 {
        align(right, text(size: 0.8em, fill: luma(110), chapters.last().body))
      }
    },
    footer: context align(center, text(size: 0.8em, fill: luma(110), counter(page).display())),
  )

  set heading(numbering: (..nums) => if nums.pos().len() <= 2 { numbering("1.1", ..nums) })
  show heading.where(level: 1): it => {
    pagebreak(weak: true)
    context {
      let pending = guide-pending-part.get()
      if pending != none {
        text(size: 0.8em, weight: "bold", fill: guide-accent, tracking: 0.06em, upper(pending))
        v(0.2em)
      }
    }
    guide-pending-part.update(none)
    set par(justify: false)
    block(below: 1.4em, {
      if it.numbering != none {
        text(size: 2.2em, weight: "bold", fill: guide-accent, counter(heading).display("1"))
        h(0.5em)
      }
      text(size: 1.9em, weight: "bold", it.body)
      v(-0.4em)
      line(length: 100%, stroke: 0.8pt + guide-accent)
    })
  }
  show heading.where(level: 2): it => {
    set par(justify: false)
    block(above: 1.6em, below: 0.7em, sticky: true, text(size: 1.25em, weight: "bold", {
      if it.numbering != none {
        text(fill: guide-accent, counter(heading).display())
        h(0.6em)
      }
      it.body
    }))
  }
  show link: set text(fill: guide-accent.darken(15%))

  text(size: 2.4em, weight: "bold", title)
  if subtitle != none { block(above: 0.8em, text(size: 1.3em, fill: luma(70), subtitle)) }
  block(above: 1.2em, text(fill: luma(70), [#author #h(0.6em) · #h(0.6em) #date]))
  if build != none { block(above: 0.45em, text(size: 0.8em, fill: luma(110), build)) }
  if site != none {
    block(above: 1.6em, below: 2.4em, width: 100%, inset: 12pt, radius: 3pt, fill: guide-accent.lighten(90%), stroke: (left: 2.5pt + guide-accent))[
      *Read it online.* This PDF is a snapshot. The guide lives at #link(site), with search,
      dark mode and always the latest version; prefer it over this file.
      #if repo != none [The example app and the guide's source are at #link(repo).]
    ]
  }

  text(size: 1.6em, weight: "bold")[Contents]
  v(0.6em)
  guide-contents()

  body
}

// Pandoc writes `//` as `/\/` so that Typst markup does not read a comment; strings need it back.
#show: book.with(
$if(title)$
  title: [$title$],
$endif$
$if(subtitle)$
  subtitle: [$subtitle$],
$endif$
$if(by-author)$
  author: "$for(by-author)$$it.name.literal$$sep$, $endfor$",
$endif$
$if(date)$
  date: "$date$",
$endif$
$if(book.site-url)$
  site: "$book.site-url$".replace("\\/", "/"),
$endif$
$if(book.repo-url)$
  repo: "$book.repo-url$".replace("\\/", "/"),
$endif$
$if(build-sha)$
  build: [Source $if(build-tag)$$build-tag$ · $endif$$build-sha$],
$endif$
$if(fontsize)$
  font-size: $fontsize$,
$endif$
)
