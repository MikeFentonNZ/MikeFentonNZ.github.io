# Michael Fenton — Academic Portfolio (GitHub Pages)

Public URL: **https://MikeFentonNZ.github.io**

## Deploy
1. Create the repository **MikeFentonNZ.github.io**.
2. Upload the contents of this folder to the repo root (or push via git).
3. GitHub Pages will auto-publish.

> If you intend to use the *GitHub Academic Pages (Jekyll)* theme instead of this static site, **do not add a `.nojekyll` file** (and integrate these assets into your Jekyll `_includes` / `_layouts`). This bundle is ready as a static site.

## Customize
- Update social profile URLs in the sidebar and in the JSON-LD `sameAs` array.
- Replace files in `/files/` with your syllabus, CV, decks.
- Edit `sitemap.xml` if you add pages.

## Notes
- Dark mode toggle is implemented via `data-theme` attribute (`assets/js/main.js`) and CSS variable sets in `assets/css/style.css` under `:root[data-theme="dark"]` and `:root[data-theme="light"]`.
- Mobile nav uses single-row horizontal scroll.
