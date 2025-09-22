// Mobile nav toggle
const toggle = document.querySelector('.nav-toggle');
const nav = document.querySelector('#mainnav');
if (toggle && nav) {
  toggle.addEventListener('click', () => {
    const isOpen = nav.classList.toggle('open');
    toggle.setAttribute('aria-expanded', String(isOpen));
  });
}

// Theme manager (works even without initial data-theme)
(function(){
  const root = document.documentElement;
  const btn = document.querySelector('#themeToggle');
  const storageKey = 'theme-pref';
  const getPreferred = () => {
    const stored = localStorage.getItem(storageKey);
    if (stored === 'light' || stored === 'dark') return stored;
    return window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
  };
  const apply = (theme, persist=true) => {
    root.setAttribute('data-theme', theme);
    if (persist) localStorage.setItem(storageKey, theme);
    if (btn) {
      const isDark = theme === 'dark';
      btn.setAttribute('aria-pressed', String(isDark));
      const icon = btn.querySelector('.icon');
      const label = btn.querySelector('.label');
      if (icon) icon.textContent = isDark ? '🌙' : '☀️';
      if (label) label.textContent = isDark ? 'Dark' : 'Light';
      btn.setAttribute('aria-label', `Activate ${isDark ? 'light' : 'dark'} mode`);
      btn.title = `Switch to ${isDark ? 'light' : 'dark'} mode`;
    }
  };
  apply(getPreferred(), false);
  const media = window.matchMedia('(prefers-color-scheme: dark)');
  const mediaHandler = (e) => {
    const stored = localStorage.getItem(storageKey);
    if (stored !== 'light' && stored !== 'dark') apply(e.matches ? 'dark' : 'light', false);
  };
  if (media && media.addEventListener) media.addEventListener('change', mediaHandler);
  if (btn) btn.addEventListener('click', () => {
    const current = root.getAttribute('data-theme') || getPreferred();
    apply(current === 'dark' ? 'light' : 'dark');
  });
})();

// Active nav highlighting
(function(){
  const page = document.documentElement.getAttribute('data-page');
  if (!page) return;
  const map = {home:['index.html','./',''],teaching:['teaching.html'],projects:['projects.html'],talks:['talks.html'],workshops:['workshops.html'],publications:['publications.html'],cv:['cv.html']};
  const targets = map[page]||[];
  document.querySelectorAll('.mainnav a').forEach(a=>{
    const href = (a.getAttribute('href')||'').toLowerCase();
    if (targets.some(t=>href.endsWith(t))) {
      a.classList.add('is-active');
      a.setAttribute('aria-current','page');
    }
  });
})();

// Footer year
const yearEl = document.getElementById('year');
if (yearEl) yearEl.textContent = new Date().getFullYear();
