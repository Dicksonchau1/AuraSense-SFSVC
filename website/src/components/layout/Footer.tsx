import { Link } from 'react-router-dom';

export function Footer() {
  return (
    <footer style={{
      borderTop: '1px solid var(--color-border-subtle)',
      padding: '3rem 1.5rem 2rem',
      fontSize: '0.75rem',
      color: 'var(--color-text-muted)',
    }}>
      <div style={{
        maxWidth: '1200px',
        margin: '0 auto',
        display: 'flex',
        justifyContent: 'space-between',
        flexWrap: 'wrap',
        gap: '2rem',
      }}>
        <div>
          <div style={{ fontWeight: 700, color: 'var(--color-text-primary)', marginBottom: '0.5rem' }}>
            ⚡ NEPA
          </div>
          <div>© {new Date().getFullYear()} AuraSense HK. All rights reserved.</div>
        </div>
        <div style={{ display: 'flex', gap: '1.5rem' }}>
          <Link to="/platform" style={{ color: 'var(--color-text-muted)', textDecoration: 'none' }}>Platform</Link>
          <Link to="/products" style={{ color: 'var(--color-text-muted)', textDecoration: 'none' }}>Products</Link>
          <Link to="/careers" style={{ color: 'var(--color-text-muted)', textDecoration: 'none' }}>Careers</Link>
          <Link to="/contact" style={{ color: 'var(--color-text-muted)', textDecoration: 'none' }}>Contact</Link>
        </div>
      </div>
    </footer>
  );
}
